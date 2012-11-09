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

#ifndef BERRYQTWORKBENCHTWEAKLET_H_
#define BERRYQTWORKBENCHTWEAKLET_H_

#include <berryWorkbenchTweaklet.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

class BERRY_UI_QT QtWorkbenchTweaklet : public QObject, public WorkbenchTweaklet
{
  Q_OBJECT
  Q_INTERFACES(berry::WorkbenchTweaklet)

public:

  berryObjectMacro(QtWorkbenchTweaklet);

  QtWorkbenchTweaklet();
  QtWorkbenchTweaklet(const QtWorkbenchTweaklet& other);

  Display* CreateDisplay();

  bool IsRunning();

  SmartPointer<WorkbenchWindow> CreateWorkbenchWindow(int number);

  void* CreatePageComposite(void* parent);

  IDialog::Pointer CreateStandardDialog(const std::string& id);

};

} // namespace berry

#endif /*BERRYQTWORKBENCHTWEAKLET_H_*/
