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

namespace cherry {

class Shell;

class QtAbstractControlWidget
{

public:

  void AddControlListener(GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(GuiTk::IControlListener::Pointer listener);

  void AddShellListener(IShellListener::Pointer listener);
  void RemoveShellListener(IShellListener::Pointer listener);

  virtual Shell* GetShell() = 0;

protected:

  GuiTk::IControlListener::Events controlEvents;
  IShellListener::Events shellEvents;

};

}

#endif /* CHERRYIQTCONTROLWIDGET_H_ */
