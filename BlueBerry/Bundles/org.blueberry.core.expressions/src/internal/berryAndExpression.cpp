/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center, 
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without 
even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryAndExpression.h"

namespace berry
{

bool AndExpression::operator==(Expression& object)
{
  try
  {
    AndExpression& that = dynamic_cast<AndExpression&>(object);
    return this->Equals(this->fExpressions, that.fExpressions);
  }
  catch (std::bad_cast exc)
  {
    return false;
  }

}

EvaluationResult
AndExpression::Evaluate(IEvaluationContext* context)
{
  return this->EvaluateAnd(context);
}

} // namespace berry
