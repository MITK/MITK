/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIEVALUATIONRESULTCACHE_H_
#define BERRYIEVALUATIONRESULTCACHE_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct IEvaluationContext;
class Expression;

/**
 * <p>
 * A cache of the result of an expression. This also provides the source
 * priority for the expression.
 * </p>
 * <p>
 * This interface is not intended to be implemented or extended by clients.
 * </p>
 *
 * @see ISources
 * @see ISourceProvider
 */
struct BERRY_UI_QT IEvaluationResultCache : public virtual Object
{

  berryObjectMacro(berry::IEvaluationResultCache);

  ~IEvaluationResultCache() override;

  /**
   * Clears the cached computation of the <code>evaluate</code> method, if
   * any. This method is only intended for internal use. It provides a
   * mechanism by which <code>ISourceProvider</code> events can invalidate
   * state on a <code>IEvaluationResultCache</code> instance.
   */
  virtual void ClearResult() = 0;

  /**
   * Returns the expression controlling the activation or visibility of this
   * item.
   *
   * @return The expression associated with this item; may be
   *         <code>null</code>.
   */
  virtual SmartPointer<Expression> GetExpression() const = 0;

  /**
   * Returns the priority that has been given to this expression.
   *
   * @return The priority.
   * @see ISources
   */
  virtual int GetSourcePriority() const = 0;

  /**
   * Evaluates the expression -- given the current state of the workbench.
   * This method should cache its computation. The cache will be cleared by a
   * call to <code>clearResult</code>.
   *
   * @param context
   *            The context in which this state should be evaluated; must not
   *            be <code>null</code>.
   * @return <code>true</code> if the expression currently evaluates to
   *         <code>true</code>; <code>false</code> otherwise.
   */
  virtual bool Evaluate(IEvaluationContext* context) const = 0;

  /**
   * Forces the cached result to be a particular value. This will <b>not</b>
   * notify any users of the cache that it has changed.
   *
   * @param result
   *            The cached result to use.
   */
  virtual void SetResult(bool result) = 0;
};

}

#endif /* BERRYIEVALUATIONRESULTCACHE_H_ */
