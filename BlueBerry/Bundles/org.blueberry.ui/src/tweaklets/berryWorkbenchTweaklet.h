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


#ifndef BERRYWORKBENCHTWEAKLET_H_
#define BERRYWORKBENCHTWEAKLET_H_

#include "internal/berryTweaklets.h"

#include "berryShell.h"
#include "berryDisplay.h"
#include "dialogs/berryIDialog.h"

namespace berry
{

class WorkbenchWindow;

struct BERRY_UI WorkbenchTweaklet
{

  static Tweaklets::TweakKey<WorkbenchTweaklet> KEY;

  static const std::string DIALOG_ID_SHOW_VIEW; // = "org.blueberry.ui.dialogs.showview";

  virtual Display* CreateDisplay() = 0;

  virtual bool IsRunning() = 0;

  virtual SmartPointer<WorkbenchWindow> CreateWorkbenchWindow(int number) = 0;

  virtual IDialog::Pointer CreateStandardDialog(const std::string& id) = 0;

};

}

Q_DECLARE_INTERFACE(berry::WorkbenchTweaklet, "org.blueberry.WorkbenchTweaklet")

#endif /* BERRYWORKBENCHTWEAKLET_H_ */
