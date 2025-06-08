/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataNodeRenameAction_h
#define QmitkDataNodeRenameAction_h

#include "QmitkAbstractDataNodeAction.h"
#include <org_mitk_gui_qt_application_Export.h>

#include <QAction>

class MITK_QT_APP QmitkDataNodeRenameAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:
  QmitkDataNodeRenameAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeRenameAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private:
  void InitializeAction() override;
  void OnActionTriggered(bool checked);

  QWidget* m_Parent;
};

#endif
