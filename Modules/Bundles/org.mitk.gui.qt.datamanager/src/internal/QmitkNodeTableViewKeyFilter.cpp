/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
    berry::IPreferences::Pointer nodeTableKeyPrefs = prefService->GetSystemPreferences()->Node("/DataManager/Hotkeys");

    QKeySequence _MakeAllInvisible = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Make all nodes invisible", "Ctrl+, V")));
    QKeySequence _ToggleVisibility = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Toggle visibility of selected nodes", "V")));
    QKeySequence _DeleteSelectedNodes = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Delete selected nodes", "Del")));
    QKeySequence _Reinit = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Reinit selected nodes", "R")));
    QKeySequence _GlobalReinit = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Global Reinit", "Ctrl+, R")));
    QKeySequence _Save = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Save selected nodes", "Ctrl+, S")));
    QKeySequence _Load = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Load", "Ctrl+, L")));
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
    else if(_KeySequence == _Save)
    {
      _DataManagerView->SaveSelectedNodes(true);
      return true;
    }    
    else if(_KeySequence == _Load)
    {
      _DataManagerView->Load(true);
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
