/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataNodeRenameAction.h"

#include <QInputDialog>
#include <QWidget>

QmitkDataNodeRenameAction::QmitkDataNodeRenameAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent),
    QmitkAbstractDataNodeAction(workbenchpartSite),
    m_Parent(parent)
{
  this->setText(tr("Rename"));
  this->InitializeAction();
}

QmitkDataNodeRenameAction::QmitkDataNodeRenameAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QmitkDataNodeRenameAction(parent, berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
}

void QmitkDataNodeRenameAction::InitializeAction()
{
  connect(this, &QmitkDataNodeRenameAction::triggered, this, &QmitkDataNodeRenameAction::OnActionTriggered);
}

void QmitkDataNodeRenameAction::OnActionTriggered(bool /*checked*/)
{
  auto workbenchPartSite = m_WorkbenchPartSite.Lock();

  if (workbenchPartSite.IsNull())
    return;

  if (m_DataStorage.IsExpired())
    return;

  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
    return;

  auto selectedNodes = this->GetSelectedNodes();

  for (auto& node : selectedNodes)
  {
    if (node.IsNull())
      continue;

    bool ok = false;

    auto newName = QInputDialog::getText(
      m_Parent,
      "Rename",
      "New name:",
      QLineEdit::Normal,
      QString::fromStdString(node->GetName()),
      &ok);

    if (ok && !newName.isEmpty())
      node->SetName(newName.toStdString());
  }
}
