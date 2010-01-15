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


#ifndef CHERRYEVALUATIONRESULTCACHE_H_
#define CHERRYEVALUATIONRESULTCACHE_H_

#include "cherryIEvaluationResultCache.h"
#include <cherryEvaluationResult.h>
#include <cherryIEvaluationContext.h>
#include <cherryExpression.h>

namespace cherry {

/**
 * <p>
 * A token representing the activation or contribution of some expression-based
 * element. This caches the evaluation result so that it is only re-computed as
 * necessary.
 * </p>
 *
 * @since 3.2
 */
class EvaluationResultCache : public IEvaluationResultCache {

private:

  /**
   * The previous computed evaluation result. If no evaluation result is
   * available, then this value is <code>null</code>.
   */
  const EvaluationResult * evaluationResult;

  /**
   * The expression to evaluate. This value may be <code>null</code>, in
   * which case the evaluation result is always <code>true</code>.
   */
  Expression::Pointer expression;

  /**
   * The priority that has been given to this expression.
   */
  int sourcePriority;


public:

  /**
   * Constructs a new instance of <code>EvaluationResultCache</code>.
   *
   * @param expression
   *            The expression that must evaluate to <code>true</code>
   *            before this handler is active. This value may be
   *            <code>null</code> if it is always active.
   * @see ISources
   */
  EvaluationResultCache(Expression::Pointer expression);

  void ClearResult();

  bool Evaluate(IEvaluationContext::Pointer context);

  Expression::Pointer GetExpression();

  int GetSourcePriority() const;

  void SetResult(bool result);

};

}

#endif /* CHERRYEVALUATIONRESULTCACHE_H_ */
