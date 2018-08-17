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

#include <QKeyEvent>
#include <QKeySequence>
#include "../QmitkDataManagerView.h"

#include "berryIPreferencesService.h"
#include "berryPlatform.h"

QmitkNodeTableViewKeyFilter::QmitkNodeTableViewKeyFilter(QObject* dataManagerView)
  : QObject(dataManagerView)
{
  m_PreferencesService = berry::Platform::GetPreferencesService();
}

bool QmitkNodeTableViewKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
  QmitkDataManagerView* dataManagerView = qobject_cast<QmitkDataManagerView*>(this->parent());
  if (event->type() == QEvent::KeyPress && dataManagerView)
  {
    berry::IPreferences::Pointer nodeTableKeyPrefs = m_PreferencesService->GetSystemPreferences()->Node("/DataManager/Hotkeys");

    QKeySequence makeAllInvisible = QKeySequence(nodeTableKeyPrefs->Get("Make all nodes invisible", "Ctrl+V"));
    QKeySequence toggleVisibility = QKeySequence(nodeTableKeyPrefs->Get("Toggle visibility of selected nodes", "V"));
    QKeySequence deleteSelectedNodes = QKeySequence(nodeTableKeyPrefs->Get("Delete selected nodes", "Del"));
    QKeySequence reinit = QKeySequence(nodeTableKeyPrefs->Get("Reinit selected nodes", "R"));
    QKeySequence globalReinit = QKeySequence(nodeTableKeyPrefs->Get("Global reinit", "Ctrl+R"));
    QKeySequence showInfo = QKeySequence(nodeTableKeyPrefs->Get("Show node information", "Ctrl+I"));

    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    QKeySequence keySequence = QKeySequence(keyEvent->modifiers() + keyEvent->key());

    if (keySequence == makeAllInvisible)
    {
      // trigger deletion of selected node(s)
      dataManagerView->MakeAllNodesInvisible(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if (keySequence == deleteSelectedNodes)
    {
      // trigger deletion of selected node(s)
      dataManagerView->RemoveSelectedNodes(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if (keySequence == toggleVisibility)
    {
      // trigger deletion of selected node(s)
      dataManagerView->ToggleVisibilityOfSelectedNodes(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if (keySequence == reinit)
    {
      dataManagerView->ReinitSelectedNodes(true);
      return true;
    }
    else if (keySequence == globalReinit)
    {
      dataManagerView->GlobalReinit(true);
      return true;
    }
    else if (keySequence == showInfo)
    {
      dataManagerView->ShowInfoDialogForSelectedNodes(true);
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}
