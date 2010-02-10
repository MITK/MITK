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


#ifndef BERRYIQTCONTROLWIDGET_H_
#define BERRYIQTCONTROLWIDGET_H_

#include <berryGuiTkIControlListener.h>
#include <berryIShellListener.h>

#include <QMetaType>

namespace berry {

class Shell;

class QtWidgetController : public Object
{

public:

  berryObjectMacro(QtWidgetController)

  static const char PROPERTY_ID[];

  QtWidgetController(Shell* shell);

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

  Shell* shell;

};

}

//TODO WeakPointer: register a weak pointer as metatype
Q_DECLARE_METATYPE(berry::QtWidgetController::Pointer)

#endif /* BERRYIQTCONTROLWIDGET_H_ */
