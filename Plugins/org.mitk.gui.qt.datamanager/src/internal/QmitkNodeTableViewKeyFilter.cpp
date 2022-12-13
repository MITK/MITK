/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNodeTableViewKeyFilter.h"
#include "../QmitkDataManagerView.h"

// mitk gui qt application plugin
#include <QmitkDataNodeGlobalReinitAction.h>
#include <QmitkDataNodeHideAllAction.h>
#include <QmitkDataNodeReinitAction.h>
#include <QmitkDataNodeRemoveAction.h>
#include <QmitkDataNodeShowDetailsAction.h>
#include <QmitkDataNodeToggleVisibilityAction.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

// qt
#include <QKeyEvent>
#include <QKeySequence>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("DataManager/Hotkeys");
  }
}

QmitkNodeTableViewKeyFilter::QmitkNodeTableViewKeyFilter(QObject *dataManagerView, mitk::DataStorage *dataStorage)
  : QObject(dataManagerView), m_DataStorage(dataStorage)
{
}

bool QmitkNodeTableViewKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
  {
    // standard event processing
    return QObject::eventFilter(obj, event);
  }

  QmitkDataManagerView *dataManagerView = qobject_cast<QmitkDataManagerView *>(this->parent());
  if (event->type() == QEvent::KeyPress && dataManagerView)
  {
    auto* prefs = GetPreferences();

    QKeySequence makeAllInvisible = QKeySequence(QString::fromStdString(prefs->Get("Make all nodes invisible", "Ctrl+V")));
    QKeySequence toggleVisibility = QKeySequence(QString::fromStdString(prefs->Get("Toggle visibility of selected nodes", "V")));
    QKeySequence deleteSelectedNodes = QKeySequence(QString::fromStdString(prefs->Get("Delete selected nodes", "Del")));
    QKeySequence reinit = QKeySequence(QString::fromStdString(prefs->Get("Reinit selected nodes", "R")));
    QKeySequence globalReinit = QKeySequence(QString::fromStdString(prefs->Get("Global reinit", "Ctrl+R")));
    QKeySequence showInfo = QKeySequence(QString::fromStdString(prefs->Get("Show node information", "Ctrl+I")));

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
