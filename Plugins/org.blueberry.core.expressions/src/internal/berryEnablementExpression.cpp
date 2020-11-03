/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
