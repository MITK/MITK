#include "QmitkNodeTableViewKeyFilter.h"

#include <QKeyEvent>
#include <QKeySequence>
#include <src/internal/QmitkDataManagerView.h>

QmitkNodeTableViewKeyFilter::QmitkNodeTableViewKeyFilter( QObject* _DataManagerView )
: QObject(_DataManagerView)
{
  m_PreferencesService = 
    cherry::Platform::GetServiceRegistry().GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);
}

bool QmitkNodeTableViewKeyFilter::eventFilter( QObject *obj, QEvent *event )
{
  QmitkDataManagerView* _DataManagerView = qobject_cast<QmitkDataManagerView*>(this->parent());
  if (event->type() == QEvent::KeyPress && _DataManagerView) 
  {
    cherry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
    cherry::IPreferences::Pointer nodeTableKeyPrefs = prefService->GetSystemPreferences()->Node("/DataManager/Hotkeys");

    QKeySequence _MakeAllInvisible = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Make all nodes invisible", "Ctrl+V")));
    QKeySequence _ToggleVisibility = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Toggle visibility of selected nodes", "V")));
    QKeySequence _DeleteSelectedNodes = QKeySequence(QString::fromStdString(nodeTableKeyPrefs->Get("Delete selected nodes", "Del")));

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    QKeySequence _KeySequence = QKeySequence(keyEvent->modifiers(), keyEvent->key());
    // if no modifier was pressed the sequence is now empty
    if(_KeySequence.isEmpty())
      _KeySequence = QKeySequence(keyEvent->key());

    if(_KeySequence == _MakeAllInvisible)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->ActionMakeAllNodesInvisible(false);
      // return true: this means the delete key event is not send to the table
      return true;
    }    
    else if(_KeySequence == _DeleteSelectedNodes)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->ActionRemoveTriggered(false);
      // return true: this means the delete key event is not send to the table
      return true;
    }    
    else if(_KeySequence == _ToggleVisibility)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->ActionToggleSelectedVisibilityTriggered(false);
      // return true: this means the delete key event is not send to the table
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}