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

  berryObjectMacro(berry::ActivePartExpression)

  /**
   * Constructs a new instance of <code>ActivePartExpression</code>
   *
   * @param activePart
   *            The part to match with the active part; may be
   *            <code>null</code>
   */
  ActivePartExpression(IWorkbenchPart* activePart);

  void CollectExpressionInfo(ExpressionInfo* info) const;

  bool operator==(const Object* object) const;

  SmartPointer<const EvaluationResult> Evaluate(IEvaluationContext* context) const;

  QString ToString() const;

protected:

  uint ComputeHashCode() const;

};

}

#endif // BERRYACTIVEPARTEXPRESSION_H
