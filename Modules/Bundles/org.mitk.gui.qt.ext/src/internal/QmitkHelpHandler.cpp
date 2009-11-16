#include "QmitkHelpHandler.h"
#include <QApplication>
#include <QKeyEvent>


QmitkHelpHandler::QmitkHelpHandler() : QObject()
  {
  qApp->installEventFilter(this);               
  }

bool QmitkHelpHandler::eventFilter(QObject *obj, QEvent *event)
  {
  if (event->type() == QEvent::KeyPress)
    {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key()==16777264) //if the F1-key is pressed...
      {
      printf("F1 pressed");
      return true;
      }
    }
  // standard event processing
  return QObject::eventFilter(obj, event);
  }