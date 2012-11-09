/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryOrExpression.h"

namespace berry {

EvaluationResult OrExpression::Evaluate(IEvaluationContext* context)
{
  return this->EvaluateOr(context);
}

bool OrExpression::operator==(Expression& object)
{
  try {
    OrExpression& that = dynamic_cast<OrExpression&>(object);
    return this->fExpressions == that.fExpressions;
  }
  catch (std::bad_cast)
  {
    return false;
  }
}

}
