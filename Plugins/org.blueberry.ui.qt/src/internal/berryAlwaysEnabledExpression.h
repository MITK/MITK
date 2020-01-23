/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYALWAYSENABLEDEXPRESSION_H
#define BERRYALWAYSENABLEDEXPRESSION_H

#include <berryExpression.h>

namespace berry {

/**
 * An expression that simply returns <code>true</code> at all times. A shared
 * instance of this expression is provided.
 */
class AlwaysEnabledExpression : public Expression
{

public:

  berryObjectMacro(berry::AlwaysEnabledExpression);

  ~AlwaysEnabledExpression() override;

  static const SmartPointer<AlwaysEnabledExpression> INSTANCE;

  /*
   * @see Expression#Evaluate(IEvaluationContext)
   */
  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

private:

  AlwaysEnabledExpression();

};

}

#endif // BERRYALWAYSENABLEDEXPRESSION_H
