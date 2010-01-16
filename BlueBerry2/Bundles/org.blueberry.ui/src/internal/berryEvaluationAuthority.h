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


#ifndef BERRYEVALUATIONAUTHORITY_H_
#define BERRYEVALUATIONAUTHORITY_H_

#include "berryExpressionAuthority.h"
#include "berryEvaluationReference.h"
#include "../services/berryIEvaluationReference.h"

#include "../berryIPropertyChangeListener.h"
#include"../berryShell.h"

#include <berryExpression.h>
#include <osgi/framework/Objects.h>
#include <berryISafeRunnable.h>

#include <Poco/HashMap.h>
#include <Poco/HashSet.h>

namespace berry {

class EvaluationAuthority : public ExpressionAuthority {

private:

  friend class EvaluationService;

  /**
   *
   */
  static const std::string COMPONENT; // = "EVALUATION"; //$NON-NLS-1$

  typedef Poco::HashSet<EvaluationReference::Pointer, Object::Hash> CacheSet;
  typedef Poco::HashMap<Expression::Pointer, CacheSet, Object::Hash> ExpressionsToCacheMap;

  /**
   * A bucket sort of the evaluation references based on source priority. Each
   * reference will appear only once per set, but may appear in multiple sets.
   * If no references are defined for a particular priority level, then the
   * array at that index will only contain <code>null</code>.
   */
  Poco::HashMap<std::string, ExpressionsToCacheMap> cachesBySourceName;

  IPropertyChangeListener::Events serviceEvents;
  int notifying;

  ObjectBool::Pointer ValueOf(bool result);

  std::vector<std::string> GetNames(IEvaluationReference::Pointer ref);

  class SafeServiceChangeRunnable : public ISafeRunnable
{

public:

  SafeServiceChangeRunnable(EvaluationAuthority* evalAuth, PropertyChangeEvent::Pointer event);

  void HandleException(const std::exception& exception);

  void Run();

private:

  EvaluationAuthority* evalAuth;
  PropertyChangeEvent::Pointer event;

};

   /**
   * This will evaluate all refs with the same expression.
   *
   * @param refs
   */
  void RefsWithSameExpression(const std::vector<EvaluationReference::Pointer>& refs);

  /**
   * @param sourceNames
   */
  void StartSourceChange(const std::vector<std::string>& sourceNames);

  /**
   * @param sourceNames
   */
  void EndSourceChange(const std::vector<std::string>& sourceNames);

   /**
   * @param ref
   * @param oldValue
   * @param newValue
   */
  void FirePropertyChange(IEvaluationReference::Pointer ref, Object::Pointer oldValue,
      Object::Pointer newValue);

  void FireServiceChange(const std::string& property,
      Object::Pointer oldValue, Object::Pointer newValue);

protected:

  /*
   * (non-Javadoc)
   *
   * @see ExpressionAuthority#SourceChanged(int)
   */
  void SourceChanged(int sourcePriority);

  /*
   * (non-Javadoc)
   *
   * @see ExpressionAuthority#SourceChanged(const std::vector<std::string>&)
   */
  void SourceChanged(const std::vector<std::string>& sourceNames);


public:

  EvaluationAuthority();

  void AddEvaluationListener(IEvaluationReference::Pointer ref);

  /**
   * @param ref
   */
  void RemoveEvaluationListener(IEvaluationReference::Pointer ref);

  /**
   * @param listener
   */
  void AddServiceListener(IPropertyChangeListener::Pointer listener);

  /**
   * @param listener
   */
  void RemoveServiceListener(IPropertyChangeListener::Pointer listener);

  /**
   * <p>
   * Bug 95792. A mechanism by which the key binding architecture can force an
   * update of the handlers (based on the active shell) before trying to
   * execute a command. This mechanism is required for GTK+ only.
   * </p>
   * <p>
   * DO NOT CALL THIS METHOD.
   * </p>
   */
//  final void updateShellKludge() {
//    updateCurrentState();
//    sourceChanged(new String[] { ISources.ACTIVE_SHELL_NAME });
//  }

  /**
   * Returns the currently active shell.
   *
   * @return The currently active shell; may be <code>null</code>.
   */
  Shell::Pointer GetActiveShell();

};

}

#endif /* BERRYEVALUATIONAUTHORITY_H_ */
