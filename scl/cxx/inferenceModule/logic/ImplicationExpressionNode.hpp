/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include "TemplateExpressionNode.hpp"

using namespace inference;

class ImplicationExpressionNode : public OperatorLogicExpressionNode
{
public:
  explicit ImplicationExpressionNode(OperandsVector & operands);
  explicit ImplicationExpressionNode(ScMemoryContext * context, OperandsVector & operands);

  LogicExpressionResult check(ScTemplateParams & params) const override;
  LogicFormulaResult compute(LogicFormulaResult & result) const override;

  LogicFormulaResult generate(Replacements & replacements) const override
  {
    return {false, false, {}};
  }

  ScAddr getFormulaTemplate() const override
  {
    return {};
  }

private:
  ScMemoryContext * context;
};
