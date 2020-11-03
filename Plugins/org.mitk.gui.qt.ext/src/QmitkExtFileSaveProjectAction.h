/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkExtFileSaveProjectAction_H_
#define QmitkExtFileSaveProjectAction_H_

#include <QAction>

#include <org_mitk_gui_qt_ext_Export.h>

#include <berrySmartPointer.h>

namespace berry {
struct IWorkbenchWindow;
}

class MITK_QT_COMMON_EXT_EXPORT QmitkExtFileSaveProjectAction : public QAction
{
  Q_OBJECT

public:

  QmitkExtFileSaveProjectAction(berry::SmartPointer<berry::IWorkbenchWindow> window);
  QmitkExtFileSaveProjectAction(berry::IWorkbenchWindow* window);

protected slots:

  void Run();

private:

  void Init(berry::IWorkbenchWindow* window);

  berry::IWorkbenchWindow* m_Window;
};


#endif /*QmitkExtFileSaveProjectAction_H_*/
