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


#ifndef CHERRYIEVALUATIONREFERENCE_H_
#define CHERRYIEVALUATIONREFERENCE_H_

#include "../cherryUiDll.h"

#include "../internal/cherryIEvaluationResultCache.h"

#include <string>

namespace cherry {

struct IPropertyChangeListener;

/**
 * A token representing a core expression and property change listener currently
 * working in the <code>IEvaluationService</code>.
 * <p>
 * Note:This is not meant to be implemented or extended by clients.
 * </p>
 *
 */
struct CHERRY_UI IEvaluationReference : public IEvaluationResultCache {

  osgiInterfaceMacro(cherry::IEvaluationReference)

  /**
   * The property change listener associated with the evaluated expression.
   *
   * @return the listener for updates.
   */
  virtual SmartPointer<IPropertyChangeListener> GetListener() = 0;

  /**
   * The property used in change notifications.
   *
   * @return the property name.
   */
  virtual std::string GetProperty() const = 0;

};


}

#endif /* CHERRYIEVALUATIONREFERENCE_H_ */
