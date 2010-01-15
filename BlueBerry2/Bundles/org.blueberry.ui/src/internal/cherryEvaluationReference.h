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


#ifndef CHERRYEVALUATIONREFERENCE_H_
#define CHERRYEVALUATIONREFERENCE_H_

#include "../services/cherryIEvaluationReference.h"
#include "../cherryIPropertyChangeListener.h"

#include "cherryEvaluationResultCache.h"
#include <cherryExpression.h>

namespace cherry {

class EvaluationReference : public IEvaluationReference
{

private:

  IPropertyChangeListener::Pointer listener;
  std::string property;
  bool postingChanges;

  EvaluationResultCache resultCache;

public:

  osgiObjectMacro(EvaluationReference)

  /**
   * @param expression
   */
  EvaluationReference(Expression::Pointer expression,
      IPropertyChangeListener::Pointer listener, const std::string& property);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.services.IEvaluationReference#getListener()
   */
  IPropertyChangeListener::Pointer GetListener();

  std::string GetProperty() const;

  /* (non-Javadoc)
   * @see org.eclipse.ui.internal.services.IEvaluationReference#setFlopping(boolean)
   */
  void SetPostingChanges(bool evaluationEnabled);

  /* (non-Javadoc)
   * @see org.eclipse.ui.internal.services.IEvaluationReference#isFlopping()
   */
  bool IsPostingChanges();

  void ClearResult();

  Expression::Pointer GetExpression();

  int GetSourcePriority() const;

  bool Evaluate(IEvaluationContext::Pointer context);

  void SetResult(bool result);
};

}

#endif /* CHERRYEVALUATIONREFERENCE_H_ */
