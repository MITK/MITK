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

QmitkNodeTableViewKeyFilter::QmitkNodeTableViewKeyFilter( QObject* _DataManagerView )
: QObject(_DataManagerView)
{
  m_PreferencesService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
}

bool QmitkNodeTableViewKeyFilter::eventFilter( QObject *obj, QEvent *event )
{
  QmitkDataManagerView* _DataManagerView = qobject_cast<QmitkDataManagerView*>(this->parent());
  if (event->type() == QEvent::KeyPress && _DataManagerView)
  {
    berry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
    berry::IPreferences::Pointer nodeTableKeyPrefs = prefService->GetSystemPreferences()->Node("/Data Manager/Hotkeys");

    QKeySequence _MakeAllInvisible = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Make all nodes invisible", "Ctrl+, V")));
    QKeySequence _ToggleVisibility = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Toggle visibility of selected nodes", "V")));
    QKeySequence _DeleteSelectedNodes = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Delete selected nodes", "Del")));
    QKeySequence _Reinit = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Reinit selected nodes", "R")));
    QKeySequence _GlobalReinit = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Global Reinit", "Ctrl+, R")));
    QKeySequence _ShowInfo = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Show Node Information", "Ctrl+, I")));

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    QKeySequence _KeySequence = QKeySequence(keyEvent->modifiers(), keyEvent->key());
    // if no modifier was pressed the sequence is now empty
    if(_KeySequence.isEmpty())
      _KeySequence = QKeySequence(keyEvent->key());

    if(_KeySequence == _MakeAllInvisible)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->MakeAllNodesInvisible(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if(_KeySequence == _DeleteSelectedNodes)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->RemoveSelectedNodes(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if(_KeySequence == _ToggleVisibility)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->ToggleVisibilityOfSelectedNodes(true);
      // return true: this means the delete key event is not send to the table
      return true;
    }
    else if(_KeySequence == _Reinit)
    {
      _DataManagerView->ReinitSelectedNodes(true);
      return true;
    }
    else if(_KeySequence == _GlobalReinit)
    {
      _DataManagerView->GlobalReinit(true);
      return true;
    }
    else if(_KeySequence == _ShowInfo)
    {
      _DataManagerView->ShowInfoDialogForSelectedNodes(true);
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}
