/*=========================================================================

Program:   openCherry Platform
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

#include "cherryWithExpression.h"

#include "cherryExpressions.h"
#include "../cherryEvaluationContext.h"

namespace cherry {

const std::string WithExpression::ATT_VARIABLE= "variable";

const intptr_t WithExpression::HASH_INITIAL= Poco::Hash<std::string>()("cherry::WithExpression");

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
WithExpression::Evaluate(IEvaluationContext* context)
{
  ExpressionVariable::Pointer variable(context->GetVariable(fVariable));
  if (variable.IsNull())
  {
    throw CoreException("Variable not defined", fVariable);
  }
  return this->EvaluateAnd(new EvaluationContext(context, variable));
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

intptr_t
WithExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
  * HASH_FACTOR + Poco::Hash<std::string>()(fVariable);
}

}
