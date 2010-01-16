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

#include "berryWithExpression.h"

#include "berryExpressions.h"
#include "../berryEvaluationContext.h"

#include <Poco/Hash.h>

namespace berry {

const std::string WithExpression::ATT_VARIABLE= "variable";

const std::size_t WithExpression::HASH_INITIAL= Poco::hash("berry::WithExpression");

WithExpression::WithExpression(IConfigurationElement::Pointer configElement)
{
  bool result = configElement->GetAttribute(ATT_VARIABLE, fVariable);
  Expressions::CheckAttribute(ATT_VARIABLE, result);
}

WithExpression::WithExpression(Poco::XML::Element* element)
{
  fVariable = element->getAttribute(ATT_VARIABLE);
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable.length()> 0);
}

WithExpression::WithExpression(const std::string& variable)
 : fVariable(variable)
{

}

bool
WithExpression::operator==(Expression& object)
{
  try
  {
    WithExpression& that = dynamic_cast<WithExpression&>(object);
    return this->fVariable == that.fVariable &&
            this->Equals(this->fExpressions, that.fExpressions);
  }
  catch (std::bad_cast)
  {
    return false;
  }
}

EvaluationResult
WithExpression::Evaluate(IEvaluationContext::Pointer context)
{
  Object::Pointer variable(context->GetVariable(fVariable));
  if (variable.IsNull())
  {
    throw CoreException("Variable not defined", fVariable);
  }
  IEvaluationContext::Pointer evalContext(new EvaluationContext(context, variable));
  return this->EvaluateAnd(evalContext);
}

void
WithExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  ExpressionInfo* other = new ExpressionInfo();
  CompositeExpression::CollectExpressionInfo(other);
  if (other->HasDefaultVariableAccess())
  {
    info->AddVariableNameAccess(fVariable);
  }
  info->MergeExceptDefaultVariable(other);
}

std::size_t
WithExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
  * HASH_FACTOR + Poco::hash(fVariable);
}

}
