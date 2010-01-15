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

#ifndef CHERRYQTSHOWVIEWACTION_H_
#define CHERRYQTSHOWVIEWACTION_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QAction>

#include <cherryIWorkbenchWindow.h>
#include <cherryIViewDescriptor.h>

//TODO should be removed later
#include "../cherryUiQtDll.h"

namespace cherry
{

class CHERRY_UI_QT QtShowViewAction : public QAction {
   Q_OBJECT
   
private:
  
  IWorkbenchWindow::Pointer m_Window;

  IViewDescriptor::Pointer m_Desc;
  
  
public:
  
  QtShowViewAction(IWorkbenchWindow::Pointer window, IViewDescriptor::Pointer desc) ;

protected slots:
  
     /**
      * Implementation of method defined on <code>IAction</code>.
      */
     void Run();
 };

}

#endif /*CHERRYQTSHOWVIEWACTION_H_*/
