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

#ifndef CHERRYQTWORKBENCH_H_
#define CHERRYQTWORKBENCH_H_

#include "org.opencherry.ui/cherryWorkbench.h"

#include "cherryQtWorkbenchWindow.h"

namespace cherry {

class CHERRY_API QtWorkbench : public Workbench
{
  
public:
  
  cherryClassMacro(QtWorkbench);
  
  IWorkbenchWindow::Pointer GetActiveWorkbenchWindow();
  IDialog::Pointer CreateDialog(const std::string& dialogid);
  IViewPart::Pointer CreateErrorViewPart(const std::string& partName, Poco::Exception exc);
  IViewPane::Pointer CreateViewPane();
  
  ~QtWorkbench();
  
protected:
  void Run();
  
private:
  
  QtWorkbenchWindow::Pointer m_QtWindow;
  
};

} // namespace cherry

#endif /*CHERRYQTWORKBENCH_H_*/
