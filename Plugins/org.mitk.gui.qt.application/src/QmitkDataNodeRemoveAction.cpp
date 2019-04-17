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

// berry
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

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

      berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
      berry::IPreferences::Pointer preferencesNode =
        prefService->GetSystemPreferences()->Node(QmitkDataNodeGlobalReinitAction::ACTION_ID);

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
  if (m_WorkbenchPartSite.Expired())
  {
    return;
  }

  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto selectedNodes = GetSelectedNodes();
  RemoveAction::Run(m_WorkbenchPartSite.Lock(), m_DataStorage.Lock(), selectedNodes, m_Parent);
}
