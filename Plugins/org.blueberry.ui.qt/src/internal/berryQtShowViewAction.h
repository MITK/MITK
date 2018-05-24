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

#ifndef BERRYQTSHOWVIEWACTION_H_
#define BERRYQTSHOWVIEWACTION_H_

#include <QAction>

#include <berryIWorkbenchWindow.h>
#include <berryIViewDescriptor.h>

//TODO should be removed later
#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

class BERRY_UI_QT QtShowViewAction : public QAction {
   Q_OBJECT

private:

  IWorkbenchWindow* m_Window;

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

#endif /*BERRYQTSHOWVIEWACTION_H_*/
