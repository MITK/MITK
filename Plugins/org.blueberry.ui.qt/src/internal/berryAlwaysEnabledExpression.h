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

  berryObjectMacro(berry::AlwaysEnabledExpression)

  ~AlwaysEnabledExpression();

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
