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
EnablementExpression::operator==(const Object* object) const
{
  if (const EnablementExpression* that = dynamic_cast<const EnablementExpression*>(object))
  {
    return this->Equals(this->fExpressions, that->fExpressions);
  }
  return false;
}

EvaluationResult::ConstPointer
EnablementExpression::Evaluate(IEvaluationContext* context) const
{
  std::clock_t start = 0;
  if (Expressions::TRACING)
    start = std::clock();

  EvaluationResult::ConstPointer result = this->EvaluateAnd(context);

  if (Expressions::TRACING)
  {
    BERRY_INFO << "[Enablement Expression] - evaluation time: " <<
        (double(std::clock() - start)/(CLOCKS_PER_SEC/1000)) << " ms.";
  }

  return result;
}

} // namespace berry
