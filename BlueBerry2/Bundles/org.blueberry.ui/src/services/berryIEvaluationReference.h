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


#ifndef BERRYIEVALUATIONREFERENCE_H_
#define BERRYIEVALUATIONREFERENCE_H_

#include "../berryUiDll.h"

#include "../internal/berryIEvaluationResultCache.h"

#include <string>

namespace berry {

struct IPropertyChangeListener;

/**
 * A token representing a core expression and property change listener currently
 * working in the <code>IEvaluationService</code>.
 * <p>
 * Note:This is not meant to be implemented or extended by clients.
 * </p>
 *
 */
struct BERRY_UI IEvaluationReference : public IEvaluationResultCache {

  osgiInterfaceMacro(berry::IEvaluationReference)

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

#endif /* BERRYIEVALUATIONREFERENCE_H_ */
