/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "DirectInferenceManager.hpp"

#include <sc-memory/sc_addr.hpp>
#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <logic/LogicExpression.hpp>

#include "utils/ContainersUtils.hpp"

using namespace inference;

DirectInferenceManager::DirectInferenceManager(ScMemoryContext * ms_context)
  : ms_context(ms_context)
{
  this->solutionTreeGenerator = std::make_unique<SolutionTreeGenerator>(ms_context);
  this->templateManager = std::make_unique<TemplateManager>(ms_context);
  this->templateSearcher = std::make_unique<TemplateSearcher>(ms_context);
}

ScAddr DirectInferenceManager::applyInference(
    const ScAddr & targetStructure,
    const ScAddr & formulasSet,
    const ScAddr & inputStructure,
    const ScAddr & outputStructure)
{
  this->inputStructure = inputStructure;
  this->outputStructure = outputStructure;
  this->targetStructure = targetStructure;

  vector<ScAddr> argumentVector;
  if (inputStructure.IsValid())
  {
    ScIterator3Ptr argumentIterator = ms_context->Iterator3(inputStructure, ScType::EdgeAccessConstPosPerm, ScType::Node);
    while (argumentIterator->Next())
      templateSearcher->addParam(argumentIterator->Get(2));
    argumentVector = utils::IteratorUtils::getAllWithType(ms_context, inputStructure, ScType::Node);
    if (argumentVector.empty())
      return this->solutionTreeGenerator->createSolution(false);
  }

  templateSearcher->setInputStructure(inputStructure);

  bool targetAchieved = isTargetAchieved(targetStructure, argumentVector);

  if (targetAchieved)
    SC_LOG_DEBUG("Target is already achieved");
  else
  {
    if (!formulasSet.IsValid())
    {
      SC_LOG_DEBUG("Formulas set is not valid");
      return this->solutionTreeGenerator->createSolution(targetAchieved);
    }

    vector<queue<ScAddr>> formulasQueuesByPriority;
    try
    {
      formulasQueuesByPriority = createFormulasQueuesListByPriority(formulasSet);
    }
    catch (std::runtime_error & exception)
    {
      SC_LOG_ERROR(exception.what());
      return this->solutionTreeGenerator->createSolution(targetAchieved);
    }

    if (formulasQueuesByPriority.empty())
    {
      SC_LOG_DEBUG("No rule sets found.");
      return this->solutionTreeGenerator->createSolution(targetAchieved);
    }

    vector<ScAddr> checkedFormulas;
    queue<ScAddr> uncheckedFormulas;

    ScAddr rule;
    ScAddr model = (inputStructure.IsValid() ? inputStructure : InferenceKeynodes::knowledge_base_IMS);
    bool isGenerated;
    SC_LOG_DEBUG("Start rule applying. There is " + to_string(formulasQueuesByPriority.size()) + " formulas sets");
    for (size_t formulasQueueIndex = 0; formulasQueueIndex < formulasQueuesByPriority.size() && !targetAchieved; formulasQueueIndex++)
    {
      uncheckedFormulas = formulasQueuesByPriority[formulasQueueIndex];
      SC_LOG_DEBUG("There is " + to_string(uncheckedFormulas.size()) + " formulas in " + to_string(formulasQueueIndex + 1) + " set");
      while (!uncheckedFormulas.empty())
      {
        rule = uncheckedFormulas.front();
        clearSatisfiabilityInformation(rule, model);
        SC_LOG_DEBUG("Trying to generate by formula: " + ms_context->HelperGetSystemIdtf(rule));
        isGenerated = useRule(rule, argumentVector);
        SC_LOG_DEBUG(std::string("Logical formulas is ") + (isGenerated ? "generated" : "not generated"));
        if (isGenerated)
        {
          addSatisfiabilityInformation(rule, model, true);
          targetAchieved = isTargetAchieved(targetStructure, argumentVector);
          if (targetAchieved)
          {
            SC_LOG_DEBUG("Target achieved");
            break;
          }
          else
          {
            ContainersUtils::addToQueue(checkedFormulas, uncheckedFormulas);
            formulasQueueIndex = 0;
            checkedFormulas.clear();
          }
        }
        else
        {
          addSatisfiabilityInformation(rule, model, false);
          checkedFormulas.push_back(rule);
        }

        uncheckedFormulas.pop();
      }
    }
  }

  return this->solutionTreeGenerator->createSolution(targetAchieved);
}

queue<ScAddr> DirectInferenceManager::createQueue(ScAddr const & set)
{
  queue<ScAddr> queue;
  vector<ScAddr> elementList = utils::IteratorUtils::getAllWithType(ms_context, set, ScType::Node);

  ContainersUtils::addToQueue(elementList, queue);
  return queue;
}

bool DirectInferenceManager::useRule(ScAddr const & rule, vector<ScAddr> /*const*/ & argumentVector)
{
  LogicFormulaResult formulaResult = {false, false, {}};
  ScAddr formulaRoot =
      utils::IteratorUtils::getAnyByOutRelation(ms_context, rule, InferenceKeynodes::rrel_main_key_sc_element);
  if (!formulaRoot.IsValid())
    return false;

  LogicExpression logicExpression(
      ms_context, templateSearcher.get(), templateManager.get(), argumentVector, outputStructure);

  auto expressionRoot = logicExpression.build(formulaRoot);
  auto result = expressionRoot->compute(formulaResult);

  return result.isGenerated;
}

vector<queue<ScAddr>> DirectInferenceManager::createFormulasQueuesListByPriority(ScAddr const & formulasSet)
{
  vector<queue<ScAddr>> formulasQueuesList;

  ScAddr setOfFormulas =
      utils::IteratorUtils::getAnyByOutRelation(ms_context, formulasSet, scAgentsCommon::CoreKeynodes::rrel_1);
  while (setOfFormulas.IsValid())
  {
    formulasQueuesList.push_back(createQueue(setOfFormulas));
    setOfFormulas = utils::IteratorUtils::getNextFromSet(ms_context, formulasSet, setOfFormulas);
  }

  return formulasQueuesList;
}

bool DirectInferenceManager::generateStatement(ScAddr const & statement, ScTemplateParams const & templateParams)
{
  bool result = false;
  ScTemplate searchTemplate;
  ms_context->HelperBuildTemplate(searchTemplate, statement, templateParams);
  ScTemplateSearchResult templateSearchResult;
  if (!ms_context->HelperSearchTemplate(searchTemplate, templateSearchResult))
  {
    ScTemplate statementTemplate;
    ms_context->HelperBuildTemplate(statementTemplate, statement);
    ScTemplateGenResult templateGenResult;
    result = ms_context->HelperGenTemplate(statementTemplate, templateGenResult, templateParams);
  }
  return result;
}

bool DirectInferenceManager::isTargetAchieved(ScAddr const & targetStructure, vector<ScAddr> const & argumentVector)
{
  auto templateParamsVector = templateManager->createTemplateParams(targetStructure, argumentVector);
  for (auto const & templateParams : templateParamsVector)
  {
    auto searchResult = templateSearcher->searchTemplate(targetStructure, templateParams);
    if (!searchResult.empty())
      return true;
  }
  return false;
}

void DirectInferenceManager::clearSatisfiabilityInformation(ScAddr const & formula, ScAddr const & model)
{
  ScIterator5Ptr iterator5Ptr = ms_context->Iterator5(
        formula, ScType::EdgeDCommon, model, ScType::EdgeAccess, InferenceKeynodes::nrel_satisfiable_formula);

  while (iterator5Ptr->Next())
    ms_context->EraseElement(iterator5Ptr->Get(1));
}

void DirectInferenceManager::addSatisfiabilityInformation(ScAddr const & formula, ScAddr const & model, bool isSatisfiable)
{
  clearSatisfiabilityInformation(formula, model);
  ScAddr satisfiableRelationEdge = ms_context->CreateEdge(ScType::EdgeDCommonConst, formula, model);
  ScType accessArcType = (isSatisfiable ? ScType::EdgeAccessConstPosTemp : ScType::EdgeAccessConstNegTemp);
  ms_context->CreateEdge(accessArcType, InferenceKeynodes::nrel_satisfiable_formula, satisfiableRelationEdge);
}
