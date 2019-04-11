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

#include "QmitkNodeTableViewKeyFilter.h"
#include "../QmitkDataManagerView.h"

// mitk gui qt application plugin
#include <QmitkDataNodeGlobalReinitAction.h>
#include <QmitkDataNodeHideAllAction.h>
#include <QmitkDataNodeReinitAction.h>
#include <QmitkDataNodeRemoveAction.h>
#include <QmitkDataNodeShowDetailsAction.h>
#include <QmitkDataNodeToggleVisibilityAction.h>

#include "berryIPreferencesService.h"
#include "berryPlatform.h"

// qt
#include <QKeyEvent>
#include <QKeySequence>

QmitkNodeTableViewKeyFilter::QmitkNodeTableViewKeyFilter(QObject *dataManagerView, mitk::DataStorage *dataStorage)
  : QObject(dataManagerView), m_DataStorage(dataStorage)
{
  m_PreferencesService = berry::Platform::GetPreferencesService();
}

bool QmitkNodeTableViewKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (m_DataStorage.IsExpired())
  {
    // standard event processing
    return QObject::eventFilter(obj, event);
  }

  auto dataStorage = m_DataStorage.Lock();

  QmitkDataManagerView *dataManagerView = qobject_cast<QmitkDataManagerView *>(this->parent());
  if (event->type() == QEvent::KeyPress && dataManagerView)
  {
    berry::IPreferences::Pointer nodeTableKeyPrefs =
      m_PreferencesService->GetSystemPreferences()->Node("/DataManager/Hotkeys");

    QKeySequence makeAllInvisible = QKeySequence(nodeTableKeyPrefs->Get("Make all nodes invisible", "Ctrl+V"));
    QKeySequence toggleVisibility = QKeySequence(nodeTableKeyPrefs->Get("Toggle visibility of selected nodes", "V"));
    QKeySequence deleteSelectedNodes = QKeySequence(nodeTableKeyPrefs->Get("Delete selected nodes", "Del"));
    QKeySequence reinit = QKeySequence(nodeTableKeyPrefs->Get("Reinit selected nodes", "R"));
    QKeySequence globalReinit = QKeySequence(nodeTableKeyPrefs->Get("Global reinit", "Ctrl+R"));
    QKeySequence showInfo = QKeySequence(nodeTableKeyPrefs->Get("Show node information", "Ctrl+I"));

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    QKeySequence keySequence = QKeySequence(keyEvent->modifiers() + keyEvent->key());
    // if no modifier was pressed the sequence is now empty
    if (keySequence.isEmpty())
    {
      keySequence = QKeySequence(keyEvent->key());
    }

    auto selectedNodes = AbstractDataNodeAction::GetSelectedNodes(dataManagerView->GetSite());
    if (keySequence == makeAllInvisible)
    {
      if (selectedNodes.empty())
      {
        // if no nodes are selected, hide all nodes of the data storage
        auto nodeset = dataStorage->GetAll();
        for (auto it = nodeset->Begin(); it != nodeset->End(); ++it)
        {
          mitk::DataNode* node = it->Value();
          if (nullptr != node)
          {
            selectedNodes.push_back(node);
          }
        }
      }

      HideAllAction::Run(selectedNodes);
      return true;
    }
    if (keySequence == deleteSelectedNodes)
    {
      RemoveAction::Run(dataManagerView->GetSite(), dataStorage, selectedNodes);
      return true;
    }
    if (keySequence == toggleVisibility)
    {
      ToggleVisibilityAction::Run(dataManagerView->GetSite(), dataStorage, selectedNodes);
      return true;
    }
    if (keySequence == reinit)
    {
      ReinitAction::Run(dataManagerView->GetSite(), dataStorage, selectedNodes);
      return true;
    }
    if (keySequence == globalReinit)
    {
      GlobalReinitAction::Run(dataManagerView->GetSite(), dataStorage);
      return true;
    }
    if (keySequence == showInfo)
    {
      ShowDetailsAction::Run(selectedNodes);
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}
