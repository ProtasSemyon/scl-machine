/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "sc-memory/kpm/sc_agent.hpp"
#include "sc-agents-common/keynodes/coreKeynodes.hpp"
#include "agent/DirectInferenceAgent.hpp"
#include <sc-agents-common/utils/AgentUtils.hpp>
#include "sc-agents-common/utils/IteratorUtils.hpp"

#include "sc_test.hpp"
#include "scs_loader.hpp"

#include "manager/DirectInferenceManager.hpp"
#include "keynodes/InferenceKeynodes.hpp"
#include "classifier/FormulaClassifier.hpp"
#include "utils/ReplacementsUtils.hpp"

using namespace inference;

namespace directInferenceLogicTest
{
ScsLoader loader;
const std::string TEST_FILES_DIR_PATH = TEMPLATE_SEARCH_MODULE_TEST_SRC_PATH "/testStructures/LogicModule/";
const std::string QUESTION_IDENTIFIER = "inference_logic_test_question";

using InferenceLogicTest = ScMemoryTest;
const int WAIT_TIME = 1500;

void initialize()
{
  InferenceKeynodes::InitGlobal();
  scAgentsCommon::CoreKeynodes::InitGlobal();

  ScAgentInit(true);
  SC_AGENT_REGISTER(inference::DirectInferenceAgent);
}

void shutdown()
{
  SC_AGENT_UNREGISTER(inference::DirectInferenceAgent);
}

// a -> b; Simple test with only one implication that must generates one class to the argument
TEST_F(InferenceLogicTest, TrueSimpleLogicRule)
{
  ScMemoryContext context(sc_access_lvl_make_min, "successful_inference");

  loader.loadScsFile(context,TEST_FILES_DIR_PATH + "trueSimpleRuleTest.scs");
  initialize();

  ScAddr action = context.HelperResolveSystemIdtf("four_arguments_action");
  EXPECT_TRUE(action.IsValid());

  ScAddr argument = context.HelperFindBySystemIdtf("argument");
  EXPECT_TRUE(argument.IsValid());

  ScIterator3Ptr argumentClassIteratorBefore = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only two classes of argument before agent run
  EXPECT_TRUE(argumentClassIteratorBefore->Next());
  EXPECT_TRUE(argumentClassIteratorBefore->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorBefore->Next());

  context.CreateEdge(
        ScType::EdgeAccessConstPosPerm,
        InferenceKeynodes::action_direct_inference,
        action);

  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, action, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
        scAgentsCommon::CoreKeynodes::question_finished_successfully,
        action,
        ScType::EdgeAccessConstPosPerm));

  ScIterator3Ptr argumentClassIteratorAfter = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only 3 classes of argument: two was before agent run and one is generated
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorAfter->Next());

  shutdown();
  context.Destroy();
}

// a -> b; Simple test with only one implication that must generates one class to the argument
TEST_F(InferenceLogicTest, TrueSimpleLogicRuleThreeArguments)
{
  ScMemoryContext context(sc_access_lvl_make_min, "successful_inference");

  loader.loadScsFile(context,TEST_FILES_DIR_PATH + "trueSimpleRuleTest.scs");
  initialize();

  ScAddr action = context.HelperResolveSystemIdtf("three_arguments_action");
  EXPECT_TRUE(action.IsValid());

  ScAddr argument = context.HelperFindBySystemIdtf("argument");
  EXPECT_TRUE(argument.IsValid());

  ScIterator3Ptr argumentClassIteratorBefore = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only two classes of argument before agent run
  EXPECT_TRUE(argumentClassIteratorBefore->Next());
  EXPECT_TRUE(argumentClassIteratorBefore->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorBefore->Next());

  context.CreateEdge(
        ScType::EdgeAccessConstPosPerm,
        InferenceKeynodes::action_direct_inference,
        action);

  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, action, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
        scAgentsCommon::CoreKeynodes::question_finished_successfully,
        action,
        ScType::EdgeAccessConstPosPerm));

  ScIterator3Ptr argumentClassIteratorAfter = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only 3 classes of argument: two was before agent run and one is generated
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorAfter->Next());

  shutdown();
  context.Destroy();
}

// a -> b; b -> c. Should apply both of them to achieve the target
TEST_F(InferenceLogicTest, TrueDoubleApplyLogicRule)
{
  ScMemoryContext context(sc_access_lvl_make_min, "successful_inference");

  loader.loadScsFile(context,TEST_FILES_DIR_PATH + "trueDoubleApplyRuleTest.scs");
  initialize();

  ScAddr action = context.HelperResolveSystemIdtf(QUESTION_IDENTIFIER);
  EXPECT_TRUE(action.IsValid());

  ScAddr argument = context.HelperFindBySystemIdtf("argument");
  EXPECT_TRUE(argument.IsValid());

  ScIterator3Ptr argumentClassIteratorBefore = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only one class of argument before agent run
  EXPECT_TRUE(argumentClassIteratorBefore->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorBefore->Next());

  context.CreateEdge(
        ScType::EdgeAccessConstPosPerm,
        InferenceKeynodes::action_direct_inference,
        action);

  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, action, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
        scAgentsCommon::CoreKeynodes::question_finished_successfully,
        action,
        ScType::EdgeAccessConstPosPerm));

  ScIterator3Ptr argumentClassIteratorAfter = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only two classes of argument: one was before agent run and two was generated
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorAfter->Next());

  shutdown();
  context.Destroy();
}

// (a -> b) -> c
TEST_F(InferenceLogicTest, TrueComplexRule)
{
  ScMemoryContext context(sc_access_lvl_make_min, "successful_inference");

  loader.loadScsFile(context,TEST_FILES_DIR_PATH + "trueComplexImplicationRule.scs");
  initialize();

  ScAddr action = context.HelperResolveSystemIdtf(QUESTION_IDENTIFIER);
  EXPECT_TRUE(action.IsValid());

  ScAddr argument = context.HelperFindBySystemIdtf("argument");
  EXPECT_TRUE(argument.IsValid());

  ScIterator3Ptr argumentClassIteratorBefore = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only one class of argument before agent run
  EXPECT_TRUE(argumentClassIteratorBefore->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorBefore->Next());

  context.CreateEdge(
        ScType::EdgeAccessConstPosPerm,
        InferenceKeynodes::action_direct_inference,
        action);

  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, action, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
        scAgentsCommon::CoreKeynodes::question_finished_successfully,
        action,
        ScType::EdgeAccessConstPosPerm));

  ScIterator3Ptr argumentClassIteratorAfter = context.Iterator3(
        ScType::NodeConstClass,
        ScType::EdgeAccessConstPosPerm,
        argument);

  // There is only two classes of argument: one was before agent run and two was generated
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());
  EXPECT_TRUE(argumentClassIteratorAfter->Next());

  // And there is no more classes
  EXPECT_FALSE(argumentClassIteratorAfter->Next());

  shutdown();
  context.Destroy();
}

TEST_F(InferenceLogicTest, TrueLogicRule)
{
  ScMemoryContext context(sc_access_lvl_make_min, "successful_inference");

  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "inferenceLogicTrueComplexRuleTest.scs");
  initialize();

  ScAddr test = context.HelperResolveSystemIdtf(QUESTION_IDENTIFIER);

  context.CreateEdge(ScType::EdgeAccessConstPosPerm, InferenceKeynodes::action_direct_inference, test);

  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, test, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
      scAgentsCommon::CoreKeynodes::question_finished_successfully, test, ScType::EdgeAccessConstPosPerm));

  shutdown();
  context.Destroy();
}

// TODO (MksmOrlov): !a -> B doesn't work
TEST_F(InferenceLogicTest, TrueComplexLogicRule)
{
  ScMemoryContext context(sc_access_lvl_make_min, "successful_inference");

  loader.loadScsFile(context,TEST_FILES_DIR_PATH + "trueComplexRuleTest.scs");
  initialize();

  ScAddr test = context.HelperResolveSystemIdtf(QUESTION_IDENTIFIER);

  context.CreateEdge(
        ScType::EdgeAccessConstPosPerm,
        InferenceKeynodes::action_direct_inference,
        test);

  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, test, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
        scAgentsCommon::CoreKeynodes::question_finished_successfully,
        test,
        ScType::EdgeAccessConstPosPerm));

  shutdown();
  context.Destroy();
}

TEST_F(InferenceLogicTest, FalseLogicRule)
{
  ScMemoryContext context(sc_access_lvl_make_min, "unsuccessful_inference");

  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "inferenceLogicFalseComplexRuleTest.scs");
  initialize();

  ScAddr test = context.HelperResolveSystemIdtf(QUESTION_IDENTIFIER);

  context.CreateEdge(ScType::EdgeAccessConstPosPerm, InferenceKeynodes::action_direct_inference, test);
  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, test, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
      scAgentsCommon::CoreKeynodes::question_finished_unsuccessfully, test, ScType::EdgeAccessConstPosPerm));

  shutdown();
  context.Destroy();
}

TEST_F(InferenceLogicTest, EquivalencesNested)
{
  ScMemoryContext context(sc_access_lvl_make_min, "testSeveralNestedEquivalences");

  auto const & name = TEST_FILES_DIR_PATH + "inferenceTestEquivalences.scs";
  struct stat buffer;
  SC_LOG_DEBUG("exists = " + to_string(stat(name.c_str(), &buffer) == 0));

  loader.loadScsFile(context, name);
  initialize();

  ScAddr test = context.HelperResolveSystemIdtf(QUESTION_IDENTIFIER);

  context.CreateEdge(ScType::EdgeAccessConstPosPerm, InferenceKeynodes::action_direct_inference, test);
  EXPECT_TRUE(utils::AgentUtils::applyAction(&context, test, WAIT_TIME));
  EXPECT_TRUE(context.HelperCheckEdge(
      scAgentsCommon::CoreKeynodes::question_finished_unsuccessfully, test, ScType::EdgeAccessConstPosPerm));

  shutdown();
  context.Destroy();
}

TEST_F(InferenceLogicTest, RuleIsImplication)
{
  ScMemoryContext context(sc_access_lvl_make_min, "implication_detected");
  FormulaClassifier fc(&context);

  loader.loadScsFile(context, TEST_FILES_DIR_PATH + "inferenceLogicTrueComplexRuleTest.scs");
  initialize();

  ScAddr testRule = context.HelperResolveSystemIdtf("inference_logic_test_rule");
  ScAddr rrel_main_key_sc_element = context.HelperResolveSystemIdtf("rrel_main_key_sc_element");
  ScIterator5Ptr iter5 = context.Iterator5(
      testRule,
      ScType::EdgeAccessConstPosPerm,
      ScType::Unknown,
      ScType::EdgeAccessConstPosPerm,
      rrel_main_key_sc_element);
  if (iter5->Next())
  {
    ScAddr formula = iter5->Get(2);
    EXPECT_EQ(fc.typeOfFormula(formula), FormulaClassifier::IMPLICATION_EDGE);
    ScAddr begin;
    ScAddr end;
    context.GetEdgeInfo(formula, begin, end);
    EXPECT_EQ(fc.typeOfFormula(begin), FormulaClassifier::CONJUNCTION);
    EXPECT_EQ(fc.typeOfFormula(end), FormulaClassifier::ATOM);
  }
  else
    SC_LOG_DEBUG("Cannot find main key sc element");

  shutdown();
  context.Destroy();
}

// TODO (MksmOrlov): test without EXPECTs
TEST_F(InferenceLogicTest, FirstIntersectionTest)
{
  ScMemoryContext context(sc_access_lvl_make_min, "try_intersect_maps");
  FormulaClassifier fc(&context);
  string first = "first";
  string second = "second";
  string third = "third";
  string fourth = "fourth";

  ScAddr rrel_1 = context.HelperResolveSystemIdtf("rrel_1");
  ScAddr rrel_2 = context.HelperResolveSystemIdtf("rrel_2");
  ScAddr rrel_3 = context.HelperResolveSystemIdtf("rrel_3");
  ScAddr rrel_4 = context.HelperResolveSystemIdtf("rrel_4");
  ScAddr rrel_5 = context.HelperResolveSystemIdtf("rrel_5");
  ScAddr rrel_6 = context.HelperResolveSystemIdtf("rrel_6");
  ScAddr rrel_7 = context.HelperResolveSystemIdtf("rrel_7");

  map<string, vector<ScAddr>> firstMap;
  map<string, vector<ScAddr>> secondMap;

  firstMap[first].push_back(rrel_1);
  firstMap[first].push_back(rrel_2);
  firstMap[first].push_back(rrel_3);

  firstMap[second].push_back(rrel_1);
  firstMap[second].push_back(rrel_2);
  firstMap[second].push_back(rrel_7);

  firstMap[third].push_back(rrel_1);
  firstMap[third].push_back(rrel_6);
  firstMap[third].push_back(rrel_3);

  secondMap[first].push_back(rrel_1);
  secondMap[first].push_back(rrel_2);
  secondMap[first].push_back(rrel_3);

  secondMap[second].push_back(rrel_1);
  secondMap[second].push_back(rrel_4);
  secondMap[second].push_back(rrel_7);

  secondMap[fourth].push_back(rrel_1);
  secondMap[fourth].push_back(rrel_5);
  secondMap[fourth].push_back(rrel_3);

  auto answer = ReplacementsUtils::uniteReplacements(firstMap, secondMap);

  SC_LOG_DEBUG("printing keys");
  for (auto const & pair : answer)
  {
    SC_LOG_DEBUG("key is " + pair.first);
    for (auto const & addr : pair.second)
      SC_LOG_DEBUG("    value is " + context.HelperGetSystemIdtf(addr));
  }

  auto smth = context.HelperResolveSystemIdtf("does not exist 122 211 long live the queen");
  SC_LOG_DEBUG("smth.IsValid = " + to_string(smth.IsValid()));
  ScTemplate searchTemplate;
  ScTemplateParams templateParams;
  templateParams.Add("rrel_1", rrel_1);
  //context.HelperBuildTemplate(searchTemplate, smth, templateParams);

  //  loader.loadScsFile(context,TEST_FILES_DIR_PATH + "inferenceLogicTrueComplexRuleTest.scs");

  //  ScAddr testRule = context.HelperResolveSystemIdtf("inference_logic_test_rule");
  //  ScAddr rrel_main_key_sc_element = context.HelperResolveSystemIdtf("rrel_main_key_sc_element");
  //  ScIterator5Ptr iter5 = context.Iterator5(testRule, ScType::EdgeAccessConstPosPerm, ScType::Unknown,
  //  ScType::EdgeAccessConstPosPerm, rrel_main_key_sc_element); if(iter5->Next())
  //  {
  //    ScAddr formula = iter5->Get(2);
  //    EXPECT_EQ(fc.typeOfFormula(formula), FormulaClassifier::IMPLICATION_EDGE);
  //    ScAddr begin;
  //    ScAddr end;
  //    context.GetEdgeInfo(formula, begin, end);
  //    EXPECT_EQ(fc.typeOfFormula(begin), FormulaClassifier::CONJUNCTION);
  //    EXPECT_EQ(fc.typeOfFormula(end), FormulaClassifier::ATOM);
  //  }
  //  else SC_LOG_DEBUG("Cannot find main key sc element");

  context.Destroy();
}

}  // namespace directInferenceLogicTest
