/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYEVALUATIONSERVICE_H_
#define BERRYEVALUATIONSERVICE_H_

#include "../services/berryIEvaluationService.h"

#include "berryEvaluationAuthority.h"

namespace berry {

class EvaluationService : public IEvaluationService {

private:

  EvaluationAuthority evaluationAuthority;

public:

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.services.IEvaluationService#addEvaluationListener(org.eclipse.core.expressions.Expression,
   *      org.eclipse.jface.util.IPropertyChangeListener, java.lang.String)
   */
  IEvaluationReference::Pointer AddEvaluationListener(Expression::Pointer expression,
      IPropertyChangeListener::Pointer listener, const std::string& property);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.services.IEvaluationService#addEvaluationReference(org.eclipse.ui.services.IEvaluationReference)
   */
  void AddEvaluationReference(IEvaluationReference::Pointer ref);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.services.IEvaluationService#removeEvaluationListener(org.eclipse.ui.internal.services.IEvaluationReference)
   */
  void RemoveEvaluationListener(IEvaluationReference::Pointer ref);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.services.IServiceWithSources#addSourceProvider(org.eclipse.ui.ISourceProvider)
   */
  void AddSourceProvider(ISourceProvider::Pointer provider);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.services.IServiceWithSources#removeSourceProvider(org.eclipse.ui.ISourceProvider)
   */
  void RemoveSourceProvider(ISourceProvider::Pointer provider);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.services.IDisposable#dispose()
   */
  void Dispose();

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.services.IEvaluationService#getCurrentState()
   */
  IEvaluationContext::ConstPointer GetCurrentState();

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.services.IEvaluationService#addServiceListener(org.eclipse.jface.util.IPropertyChangeListener)
   */
  void AddServiceListener(IPropertyChangeListener::Pointer listener);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.services.IEvaluationService#removeServiceListener(org.eclipse.jface.util.IPropertyChangeListener)
   */
  void RemoveServiceListener(IPropertyChangeListener::Pointer listener);

  /* (non-Javadoc)
   * @see org.eclipse.ui.services.IEvaluationService#requestEvaluation(java.lang.String)
   */
  void RequestEvaluation(const std::string& propertyName);

//  void updateShellKludge() {
//    evaluationAuthority.updateShellKludge();
//  }

  /**
   * <p>
   * Bug 95792. A mechanism by which the key binding architecture can force an
   * update of the handlers (based on the active shell) before trying to
   * execute a command. This mechanism is required for GTK+ only.
   * </p>
   * <p>
   * DO NOT CALL THIS METHOD.
   * </p>
   *
   * @param shell
   *            The shell that should be considered active; must not be
   *            <code>null</code>.
   */
//  void UpdateShellKludge(Shell::Pointer shell) {
//    final Shell currentActiveShell = evaluationAuthority.getActiveShell();
//    if (currentActiveShell != shell) {
//      evaluationAuthority.sourceChanged(ISources.ACTIVE_SHELL,
//          ISources.ACTIVE_SHELL_NAME, shell);
//    }
//  }
};

}

#endif /* BERRYEVALUATIONSERVICE_H_ */
