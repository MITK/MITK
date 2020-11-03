/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
