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


#ifndef CHERRYIQTCONTROLWIDGET_H_
#define CHERRYIQTCONTROLWIDGET_H_

#include <guitk/cherryGuiTkIControlListener.h>
#include <cherryIShellListener.h>

#include <QMetaType>

namespace cherry {

class Shell;

class QtWidgetController : public Object
{

public:

  cherryObjectMacro(QtWidgetController)

  static const char PROPERTY_ID[];

  QtWidgetController(SmartPointer<Shell> shell);

  void AddControlListener(GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(GuiTk::IControlListener::Pointer listener);

  void AddShellListener(IShellListener::Pointer listener);
  void RemoveShellListener(IShellListener::Pointer listener);

  SmartPointer<Shell> GetShell();

protected:

  friend class QtControlWidget;
  friend class QtMainWindowControl;

  GuiTk::IControlListener::Events controlEvents;
  IShellListener::Events shellEvents;

  //TODO WeakPointer
  SmartPointer<Shell> shell;

};

}

//TODO WeakPointer: register a weak pointer as metatype
Q_DECLARE_METATYPE(cherry::QtWidgetController::Pointer)

#endif /* CHERRYIQTCONTROLWIDGET_H_ */
