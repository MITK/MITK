/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODEREINITACTION_H
#define QMITKDATANODEREINITACTION_H

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QAction>

namespace ReinitAction
{
  MITK_QT_APP void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite,
                       mitk::DataStorage::Pointer dataStorage,
                       const QList<mitk::DataNode::Pointer>& selectedNodes = QList<mitk::DataNode::Pointer>(),
                       mitk::BaseRenderer* baseRenderer = nullptr);
}

class MITK_QT_APP QmitkDataNodeReinitAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkDataNodeReinitAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeReinitAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  void InitializeAction() override;

};

#endif // QMITKDATANODEREINITACTION_H
