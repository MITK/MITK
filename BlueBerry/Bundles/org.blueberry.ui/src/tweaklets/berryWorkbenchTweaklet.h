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


#ifndef BERRYWORKBENCHTWEAKLET_H_
#define BERRYWORKBENCHTWEAKLET_H_

#include "../internal/berryTweaklets.h"

#include "../berryShell.h"
#include "../berryDisplay.h"
#include "../dialogs/berryIDialog.h"

namespace berry
{

class WorkbenchWindow;

struct BERRY_UI WorkbenchTweaklet : public Object
{
  berryInterfaceMacro(WorkbenchTweaklet, berry);

  static Tweaklets::TweakKey<WorkbenchTweaklet> KEY;

  static const std::string DIALOG_ID_SHOW_VIEW; // = "org.blueberry.ui.dialogs.showview";

  virtual Display* CreateDisplay() = 0;

  virtual bool IsRunning() = 0;

  virtual SmartPointer<WorkbenchWindow> CreateWorkbenchWindow(int number) = 0;

  virtual IDialog::Pointer CreateStandardDialog(const std::string& id) = 0;

};

}

#endif /* BERRYWORKBENCHTWEAKLET_H_ */
