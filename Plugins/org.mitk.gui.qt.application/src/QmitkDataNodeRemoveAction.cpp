/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataNodeRemoveAction.h>
#include <QmitkDataNodeGlobalReinitAction.h>

// qt
#include <QMessageBox>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace RemoveAction
{
  void Run(berry::IWorkbenchPartSite::Pointer workbenchPartSite, mitk::DataStorage::Pointer dataStorage, const QList<mitk::DataNode::Pointer>& selectedNodes, QWidget* parent /* = nullptr*/)
  {
    if (selectedNodes.empty())
    {
      return;
    }

    QString question("Do you really want to remove ");
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
    question.append(" from data storage?");

    QMessageBox::StandardButton answerButton =
      QMessageBox::question(parent, "DataManager", question, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (answerButton == QMessageBox::Yes)
    {
      for (auto& dataNode : selectedNodes)
      {
        if (nullptr == dataNode)
        {
          continue;
        }

        dataStorage->Remove(dataNode);
      }

      auto* prefService = mitk::CoreServices::GetPreferencesService();
      auto* preferencesNode = prefService->GetSystemPreferences()->Node(QmitkDataNodeGlobalReinitAction::ACTION_ID.toStdString());

      bool globalReinit = preferencesNode->GetBool("Call global reinit if node is deleted", true);
      if (globalReinit)
      {
        GlobalReinitAction::Run(workbenchPartSite, dataStorage);
      }
    }
  }
}

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

void QmitkDataNodeRemoveAction::InitializeAction()
{
  connect(this, &QmitkDataNodeRemoveAction::triggered, this, &QmitkDataNodeRemoveAction::OnActionTriggered);
}

void QmitkDataNodeRemoveAction::OnActionTriggered(bool /*checked*/)
{
  auto workbenchPartSite = m_WorkbenchPartSite.Lock();

  if (workbenchPartSite.IsNull())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
  {
    return;
  }

  auto selectedNodes = GetSelectedNodes();
  RemoveAction::Run(workbenchPartSite, dataStorage, selectedNodes, m_Parent);
}
