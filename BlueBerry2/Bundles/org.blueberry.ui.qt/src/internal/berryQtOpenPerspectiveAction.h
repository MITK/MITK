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

#ifndef BERRYQTOPENPERSPECTIVEACTION_H_
#define BERRYQTOPENPERSPECTIVEACTION_H_

#include <QAction>
#include <QActionGroup>

#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveDescriptor.h>

//TODO should be removed later
#include "../berryUiQtDll.h"

namespace berry
{

class BERRY_UI_QT QtOpenPerspectiveAction: public QAction
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

#endif /* BERRYQTOPENPERSPECTIVEACTION_H_ */
