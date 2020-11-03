/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHWINDOWEXPRESSION_H
#define BERRYWORKBENCHWINDOWEXPRESSION_H

#include "berryExpression.h"

namespace berry {

struct IWorkbenchWindow;

/**
 * <p>
 * An expression that evaluates to {@link EvaluationResult#TRUE} when the active
 * workbench window matches the window held by this expression.
 * </p>
 */
class WorkbenchWindowExpression : public Expression
{

private:

  /**
   * The seed for the hash code for all schemes.
   */
  static const int HASH_INITIAL;

  /**
   * The workbench window that must be active for this expression to evaluate
   * to <code>true</code>. If this value is <code>null</code>, then any
   * workbench window may be active.
   */
  IWorkbenchWindow* const window;

public:

  berryObjectMacro(berry::WorkbenchWindowExpression);

  /**
   * Constructs a new instance.
   *
   * @param window
   *            The workbench window which must be active for this expression
   *            to evaluate to <code>true</code>; may be <code>null</code>
   *            if this expression is always <code>true</code>.
   */
  WorkbenchWindowExpression(IWorkbenchWindow* window);

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;

  SmartPointer<const EvaluationResult> Evaluate(IEvaluationContext* context) const override;

  QString ToString() const override;

protected:

  uint ComputeHashCode() const override;

  /**
   * Returns the workbench window to which this expression applies.
   *
   * @return The workbench window to which this expression applies; may be
   *         <code>null</code>.
   */
  IWorkbenchWindow* GetWindow() const;

};

}

#endif // BERRYWORKBENCHWINDOWEXPRESSION_H
