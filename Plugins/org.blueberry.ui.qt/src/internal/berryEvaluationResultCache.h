/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEVALUATIONRESULTCACHE_H
#define BERRYEVALUATIONRESULTCACHE_H

#include "berryIEvaluationResultCache.h"

namespace berry {

class EvaluationResult;

/**
 * <p>
 * A token representing the activation or contribution of some expression-based
 * element. This caches the evaluation result so that it is only re-computed as
 * necessary.
 * </p>
 */
class EvaluationResultCache : public virtual IEvaluationResultCache
{

private:

  /**
   * The previous computed evaluation result. If no evaluation result is
   * available, then this value is <code>null</code>.
   */
  mutable SmartPointer<const EvaluationResult> evaluationResult;

  /**
   * The expression to evaluate. This value may be <code>null</code>, in
   * which case the evaluation result is always <code>true</code>.
   */
  const SmartPointer<Expression> expression;

  /**
   * The priority that has been given to this expression.
   */
  const int sourcePriority;

protected:

  /**
   * Constructs a new instance of <code>EvaluationResultCache</code>.
   *
   * @param expression
   *            The expression that must evaluate to <code>true</code>
   *            before this handler is active. This value may be
   *            <code>null</code> if it is always active.
   * @see ISources
   */
  EvaluationResultCache(const SmartPointer<Expression>& expression);

public:

  ~EvaluationResultCache() override;

  void ClearResult() override;

  bool Evaluate(IEvaluationContext* context) const override;

  SmartPointer<Expression> GetExpression() const override;

  int GetSourcePriority() const override;

  void SetResult(bool result) override;
};

}

#endif // BERRYEVALUATIONRESULTCACHE_H
