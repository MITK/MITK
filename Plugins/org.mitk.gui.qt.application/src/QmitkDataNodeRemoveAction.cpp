/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <QmitkDataNodeRemoveAction.h>
#include <QmitkDataNodeGlobalReinitAction.h>

// qt
#include <QMessageBox>

QmitkDataNodeRemoveAction::QmitkDataNodeRemoveAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  setText(tr("Remove"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeRemoveAction::QmitkDataNodeRemoveAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  setText(tr("Remove"));
  m_Parent = parent;
  InitializeAction();
}

QmitkDataNodeRemoveAction::~QmitkDataNodeRemoveAction()
{
  // nothing here
}

void QmitkDataNodeRemoveAction::InitializeAction()
{
  connect(this, &QmitkDataNodeRemoveAction::triggered, this, &QmitkDataNodeRemoveAction::OnActionTriggered);
}

void QmitkDataNodeRemoveAction::OnActionTriggered(bool checked)
{
  QString question = tr("Do you really want to remove ");
  auto selectedNodes = GetSelectedNodes();
  for (auto& dataNode : selectedNodes)
  {
    if (nullptr == dataNode)
    {
      continue;
    }

    question.append(QString::fromStdString(dataNode->GetName()));
    question.append(", ");
  }

  // remove the last two characters = ", "
  question = question.remove(question.size() - 2, 2);
  question.append(tr(" from data storage?"));

  QMessageBox::StandardButton answerButton = QMessageBox::question(m_Parent, tr("DataManager"), question,
    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    for (auto& dataNode : selectedNodes)
    {
      if (nullptr == dataNode)
      {
        continue;
      }

      if (m_DataStorage.IsExpired())
      {
        return;
      }

      auto dataStorage = m_DataStorage.Lock();
      dataStorage->Remove(dataNode);
      if (true)//m_GlobalReinitOnNodeDelete)
      {
        GlobalReinitAction::Run(m_WorkbenchPartSite.Lock(), m_DataStorage.Lock());
      }
    }
  }
}
