/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYIQTCONTROLWIDGET_H_
#define BERRYIQTCONTROLWIDGET_H_

#include <berryGuiTkIControlListener.h>
#include <berryIShellListener.h>

#include <QMetaType>

#include <org_blueberry_ui_Export.h>

namespace berry {

class Shell;

class BERRY_UI QtWidgetController : public Object
{

public:

  berryObjectMacro(QtWidgetController);

  static const char PROPERTY_ID[];

  QtWidgetController(Shell* shell);

  ~QtWidgetController();

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

  void ShellDestroyed();

  Shell* shell;

};

}

//TODO WeakPointer: register a weak pointer as metatype
Q_DECLARE_METATYPE(berry::QtWidgetController::Pointer)

#endif /* BERRYIQTCONTROLWIDGET_H_ */
