#include "QmitkDelKeyFilter.h"

#include <QKeyEvent>
#include <src/internal/QmitkDataManagerView.h>

QmitkDelKeyFilter::QmitkDelKeyFilter( QObject* _DataManagerView )
: QObject(_DataManagerView)
{

}

bool QmitkDelKeyFilter::eventFilter( QObject *obj, QEvent *event )
{
  if (event->type() == QEvent::KeyPress) 
  {
    QmitkDataManagerView* _DataManagerView = qobject_cast<QmitkDataManagerView*>(this->parent());
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if(keyEvent->key() == Qt::Key_Delete && _DataManagerView)
    {
      // trigger deletion of selected node(s)
      _DataManagerView->ActionRemoveTriggered(false);
      // return true: this means the delete key event is not send to the table
      return true;
    }
  }
  // standard event processing
  return QObject::eventFilter(obj, event);
}