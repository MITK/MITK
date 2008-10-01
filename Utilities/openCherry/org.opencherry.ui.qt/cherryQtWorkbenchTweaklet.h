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

#ifndef CHERRYQTWORKBENCHTWEAKLET_H_
#define CHERRYQTWORKBENCHTWEAKLET_H_

#include <tweaklets/cherryWorkbenchTweaklet.h>

#include "cherryUiQtDll.h"

namespace cherry {

class CHERRY_UI_QT QtWorkbenchTweaklet : public WorkbenchTweaklet
{

public:

  cherryClassMacro(QtWorkbenchTweaklet);

  int RunEventLoop();
  bool IsRunning();

  void* CreatePageComposite(void* parent);

  IDialog::Pointer CreateStandardDialog(const std::string& id);

};

} // namespace cherry

#endif /*CHERRYQTWORKBENCHTWEAKLET_H_*/
