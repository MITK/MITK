/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_AND_EXPRESSION_H__
#define __BERRY_AND_EXPRESSION_H__

#include "berryCompositeExpression.h"

namespace berry {

// Copied from org.blueberry.core.commands
class AndExpression : public CompositeExpression {

public:

  bool operator==(const Object* object) const override;

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;
};

}  // namespace berry

#endif // __BERRY_AND_EXPRESSION_H__
