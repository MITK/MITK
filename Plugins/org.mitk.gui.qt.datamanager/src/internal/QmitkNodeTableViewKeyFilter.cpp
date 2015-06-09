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

QmitkNodeTableViewKeyFilter::QmitkNodeTableViewKeyFilter( QObject* _DataManagerView )
: QObject(_DataManagerView)
{
  m_PreferencesService = berry::Platform::GetPreferencesService();
}

bool QmitkNodeTableViewKeyFilter::eventFilter( QObject *obj, QEvent *event )
{
  QmitkDataManagerView* _DataManagerView = qobject_cast<QmitkDataManagerView*>(this->parent());
  if (event->type() == QEvent::KeyPress && _DataManagerView)
  {
    berry::IPreferences::Pointer nodeTableKeyPrefs = m_PreferencesService->GetSystemPreferences()->Node("/DataManager/Hotkeys");

    QKeySequence _MakeAllInvisible = QKeySequence(nodeTableKeyPrefs->Get("Make all nodes invisible", "Ctrl+, V"));
    QKeySequence _ToggleVisibility = QKeySequence(nodeTableKeyPrefs->Get("Toggle visibility of selected nodes", "V"));
    QKeySequence _DeleteSelectedNodes = QKeySequence(nodeTableKeyPrefs->Get("Delete selected nodes", "Del"));
    QKeySequence _Reinit = QKeySequence(nodeTableKeyPrefs->Get("Reinit selected nodes", "R"));
    QKeySequence _GlobalReinit = QKeySequence(nodeTableKeyPrefs->Get("Global Reinit", "Ctrl+, R"));
    QKeySequence _ShowInfo = QKeySequence(nodeTableKeyPrefs->Get("Show Node Information", "Ctrl+, I"));

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
