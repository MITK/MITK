/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYACTIVEPARTEXPRESSION_H
#define BERRYACTIVEPARTEXPRESSION_H

#include "berryExpression.h"

namespace berry {

struct IWorkbenchPart;

/**
 * <p>
 * An expression that is bound to a particular part instance.
 * </p>
 * <p>
 * This class is not intended for use outside of the
 * <code>org.blueberry.ui</code> plug-in.
 * </p>
 */
class ActivePartExpression : public Expression
{

private:

  /**
   * The seed for the hash code for all schemes.
   */
  static const int HASH_INITIAL;

  /**
   * The part that must be active for this expression to evaluate to
   * <code>true</code>. This value is never <code>null</code>.
   */
  IWorkbenchPart* const activePart;

public:

  berryObjectMacro(berry::ActivePartExpression);

  /**
   * Constructs a new instance of <code>ActivePartExpression</code>
   *
   * @param activePart
   *            The part to match with the active part; may be
   *            <code>null</code>
   */
  ActivePartExpression(IWorkbenchPart* activePart);

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;

  SmartPointer<const EvaluationResult> Evaluate(IEvaluationContext* context) const override;

  QString ToString() const override;

protected:

  uint ComputeHashCode() const override;

};

}

#endif // BERRYACTIVEPARTEXPRESSION_H
