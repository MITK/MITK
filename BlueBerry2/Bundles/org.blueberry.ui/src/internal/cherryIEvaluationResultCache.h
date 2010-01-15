/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef CHERRYIEVALUATIONRESULTCACHE_H_
#define CHERRYIEVALUATIONRESULTCACHE_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

namespace cherry {

using namespace osgi::framework;

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
 * @see org.eclipse.ui.ISources
 * @see org.eclipse.ui.ISourceProvider
 */
struct IEvaluationResultCache : public Object {

  osgiInterfaceMacro(cherry::IEvaluationResultCache)

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
  virtual SmartPointer<Expression> GetExpression() = 0;

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
  virtual bool Evaluate(SmartPointer<IEvaluationContext> context) = 0;

  /**
   * Forces the cached result to be a particular value. This will <b>not</b>
   * notify any users of the cache that it has changed.
   *
   * @param result
   *            The cached result to use.
   * @since 3.3
   */
  virtual void SetResult(bool result) = 0;
};

}

#endif /* CHERRYIEVALUATIONRESULTCACHE_H_ */
