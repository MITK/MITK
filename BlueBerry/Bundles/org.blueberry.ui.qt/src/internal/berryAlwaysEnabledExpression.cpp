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


#include "berryAlwaysEnabledExpression.h"

namespace berry {

const SmartPointer<AlwaysEnabledExpression> AlwaysEnabledExpression::INSTANCE(new AlwaysEnabledExpression());

EvaluationResult::ConstPointer AlwaysEnabledExpression::Evaluate(IEvaluationContext* /*context*/) const
{
  return EvaluationResult::TRUE_EVAL;
}

AlwaysEnabledExpression::AlwaysEnabledExpression()
{
}

AlwaysEnabledExpression::~AlwaysEnabledExpression()
{
}

}
