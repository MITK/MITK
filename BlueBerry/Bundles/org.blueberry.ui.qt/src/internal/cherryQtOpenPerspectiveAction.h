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

#ifndef CHERRYQTOPENPERSPECTIVEACTION_H_
#define CHERRYQTOPENPERSPECTIVEACTION_H_

#include <QAction>
#include <QActionGroup>

#include <cherryIWorkbenchWindow.h>
#include <cherryIPerspectiveDescriptor.h>

//TODO should be removed later
#include "../cherryUiQtDll.h"

namespace cherry
{

class CHERRY_UI_QT QtOpenPerspectiveAction: public QAction
{
  Q_OBJECT

public:

  QtOpenPerspectiveAction(IWorkbenchWindow::Pointer window,
      IPerspectiveDescriptor::Pointer descr, QActionGroup* group);

protected slots:

  void Run();

private:

  IWorkbenchWindow* window;
  IPerspectiveDescriptor::Pointer descr;

};

}

#endif /* CHERRYQTOPENPERSPECTIVEACTION_H_ */
