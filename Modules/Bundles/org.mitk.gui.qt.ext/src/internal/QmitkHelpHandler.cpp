#include "QmitkHelpHandler.h"
#include <QApplication>
#include <QKeyEvent>

#include "cherryQtAssistantUtil.h"


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
      this->OpenHelpPage();
      return true;
      }
    }
  // standard event processing
  return QObject::eventFilter(obj, event);
  }

void QmitkHelpHandler::OpenHelpPage()
  {
  cherry::QtAssistantUtil::OpenAssistant("","");
  }