#include "SolutionTreeSearcher.hpp"
#include "keynodes/InferenceKeynodes.hpp"

namespace inference
{
SolutionTreeSearcher::SolutionTreeSearcher(ScMemoryContext * context)
  : context(context)
{
}

bool SolutionTreeSearcher::checkIfSolutionNodeExists(
    ScAddr const & rule,
    ScTemplateParams const & templateParams,
    ScAddrUnorderedSet const & variables)
{
  ScTemplate solutionNodeTemplate;
  ScTemplateSearchResult searchResult;
  std::string const & solutionAlias = "_solution";
  std::string const & solutionNodeAlias = "_solution_node";
  std::string const & solutionsSetAlias = "_solutions_set";
  std::string const & replacementForAlias = "_replacement_for";
  solutionNodeTemplate.Triple(
      InferenceKeynodes::concept_solution, ScType::EdgeAccessVarPosPerm, ScType::NodeVar >> solutionAlias);
  solutionNodeTemplate.Triple(solutionAlias, ScType::EdgeAccessVarPosPerm, ScType::NodeVar >> solutionNodeAlias);
  solutionNodeTemplate.Quintuple(
      solutionNodeAlias, ScType::EdgeAccessVarPosPerm, rule, ScType::EdgeAccessVarPosPerm, ScKeynodes::rrel_1);
  solutionNodeTemplate.Quintuple(
      solutionNodeAlias,
      ScType::EdgeAccessVarPosPerm,
      ScType::NodeVar >> solutionsSetAlias,
      ScType::EdgeAccessVarPosPerm,
      ScKeynodes::rrel_2);
  for (ScAddr const & variable : variables)
  {
    ScAddr replacement;
    templateParams.Get(variable, replacement);
    if (replacement.IsValid())
    {
      std::string const & pairAlias = replacementForAlias + std::to_string(variable.Hash());
      solutionNodeTemplate.Triple(solutionsSetAlias, ScType::EdgeAccessVarPosPerm, ScType::NodeVar >> pairAlias);
      solutionNodeTemplate.Quintuple(
          pairAlias, ScType::EdgeAccessVarPosPerm, replacement, ScType::EdgeAccessVarPosPerm, ScKeynodes::rrel_1);
      solutionNodeTemplate.Quintuple(
          pairAlias, ScType::EdgeAccessVarPosPerm, variable, ScType::EdgeAccessVarPosPerm, ScKeynodes::rrel_2);
      solutionNodeTemplate.Triple(variable, ScType::EdgeAccessVarPosTemp, replacement);
    }
    else
      SC_THROW_EXCEPTION(
          utils::ExceptionItemNotFound,
          "SolutionTreeSearcher: rule " << context->GetElementSystemIdentifier(rule) << " has var "
                                        << context->GetElementSystemIdentifier(variable)
                                        << " but templateParams don't have replacement for this var");
  }
  context->SearchByTemplate(solutionNodeTemplate, searchResult);
  return !searchResult.IsEmpty();
}
}  // namespace inference
