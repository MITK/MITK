/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "berryResolveExpression.h"

#include "berryExpressions.h"
#include "../berryEvaluationContext.h"

#include "Poco/Hash.h"

namespace berry {

const std::string ResolveExpression::ATT_VARIABLE= "variable";
const std::string ResolveExpression::ATT_ARGS= "args";
const std::size_t ResolveExpression::HASH_INITIAL = Poco::Hash<std::string>()("berry::ResolveExpression");

ResolveExpression::ResolveExpression(IConfigurationElement::Pointer configElement)
{
  bool result = configElement->GetAttribute(ATT_VARIABLE, fVariable);
  Expressions::CheckAttribute(ATT_VARIABLE, result);
  Expressions::GetArguments(fArgs, configElement, ATT_ARGS);
}

ResolveExpression::ResolveExpression(Poco::XML::Element* element)
{
  fVariable = element->getAttribute(ATT_VARIABLE);
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable.size()> 0);
  Expressions::GetArguments(fArgs, element, ATT_ARGS);
}

ResolveExpression::ResolveExpression(const std::string& variable, std::vector<Object::Pointer>& args)
 : fVariable(variable), fArgs(args)
{

}

EvaluationResult
ResolveExpression::Evaluate(IEvaluationContext* context)
{
  Object::Pointer variable= context->ResolveVariable(fVariable, fArgs);
  if (variable.IsNull())
  {
    throw CoreException("Variable not defined", fVariable);
  }
  EvaluationContext evalContext(context, variable);
  return this->EvaluateAnd(&evalContext);
}

void
ResolveExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  ExpressionInfo other;
  this->CompositeExpression::CollectExpressionInfo(&other);
  if (other.HasDefaultVariableAccess())
  {
    info->AddVariableNameAccess(fVariable);
  }
  info->MergeExceptDefaultVariable(&other);
}

bool
ResolveExpression::operator==(Expression& object)
{
  try
  {
    ResolveExpression& that = dynamic_cast<ResolveExpression&>(object);
    return this->fVariable == that.fVariable
    && this->Equals(this->fArgs, that.fArgs)
    && this->Equals(this->fExpressions, that.fExpressions);
  }
  catch (std::bad_cast)
  {
    return false;
  }
}

std::size_t
ResolveExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
  * HASH_FACTOR + this->HashCode(fArgs)
  * HASH_FACTOR + Poco::Hash<std::string>()(fVariable);
}

} // namespace berry
