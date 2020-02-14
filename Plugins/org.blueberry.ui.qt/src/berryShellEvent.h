/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSHELLEVENT_H_
#define BERRYSHELLEVENT_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryMacros.h>
#include <berryObject.h>

namespace berry {

class Shell;

/**
 * Instances of this class are sent as a result of
 * operations being performed on shells.
 *
 * @see ShellListener
 * @see <a href="http://www.blueberry.org/swt/">Sample code and further information</a>
 */

class BERRY_UI_QT ShellEvent : public Object {

public:

  berryObjectMacro(ShellEvent);

  /**
   * A flag indicating whether the operation should be allowed.
   * Setting this field to <code>false</code> will cancel the operation.
   */
  bool doit;

  /**
   * Constructs a new instance of this class based on the
   * information in the given untyped event.
   *
   * @param e the untyped event containing the information
   */
  ShellEvent(SmartPointer<Shell> source) ;

  SmartPointer<Shell> GetSource();

private:

  SmartPointer<Shell> source;

};

}

#endif /* BERRYSHELLEVENT_H_ */
