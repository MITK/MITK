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

#include "berryLog.h"

#include "berryEnablementExpression.h"

#include "berryExpressions.h"

#include <ctime>

namespace berry {

bool
EnablementExpression::operator==(Expression& object)
{
  try
  {
    EnablementExpression& that = dynamic_cast<EnablementExpression&>(object);
    return this->Equals(this->fExpressions, that.fExpressions);
  }
  catch (std::bad_cast)
  {
    return false;
  }

  return false;
}

EvaluationResult
EnablementExpression::Evaluate(IEvaluationContext* context)
{
  std::clock_t start = 0;
  if (Expressions::TRACING)
    start = std::clock();

  EvaluationResult result = this->EvaluateAnd(context);

  if (Expressions::TRACING)
  {
    BERRY_INFO << "[Enablement Expression] - evaluation time: " <<
        (double(std::clock() - start)/(CLOCKS_PER_SEC/1000)) << " ms.";
  }

  return result;
}

} // namespace berry
