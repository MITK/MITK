/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLabelSetJumpToAction_h
#define QmitkLabelSetJumpToAction_h

#include "QmitkAbstractDataNodeAction.h"

// qt
#include <QAction>

namespace LabelSetJumpToAction
{
  void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite,
           const mitk::DataNode* dataNode,
           mitk::BaseRenderer* baseRenderer = nullptr);
}

class QmitkLabelSetJumpToAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  QmitkLabelSetJumpToAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkLabelSetJumpToAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  void InitializeAction() override;
};

#endif
