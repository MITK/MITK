/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataNodeShowDetailsAction.h>

#include "QmitkNodeDetailsDialog.h"

namespace ShowDetailsAction
{
  void Run(const QList<mitk::DataNode::Pointer>& selectedNodes, QWidget* parent /* = nullptr*/)
  {
    if (selectedNodes.empty())
    {
      return;
    }

    QmitkNodeDetailsDialog infoDialog(selectedNodes, parent);
    infoDialog.exec();
  }
}

QmitkDataNodeShowDetailsAction::QmitkDataNodeShowDetailsAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Show details"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeShowDetailsAction::QmitkDataNodeShowDetailsAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Show details"));
  m_Parent = parent;
  InitializeAction();
}

void QmitkDataNodeShowDetailsAction::InitializeAction()
{
  connect(this, &QmitkDataNodeShowDetailsAction::triggered, this, &QmitkDataNodeShowDetailsAction::OnActionTriggered);
}

void QmitkDataNodeShowDetailsAction::OnActionTriggered(bool /*checked*/)
{
  auto selectedNodes = GetSelectedNodes();
  ShowDetailsAction::Run(selectedNodes, m_Parent);
}
