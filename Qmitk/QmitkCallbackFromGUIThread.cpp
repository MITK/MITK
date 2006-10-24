#include "QmitkCallbackFromGUIThread.h"

#include<qapplication.h>

QmitkCallbackFromGUIThread globalQmitkCallbackFromGUIThread;

/// This kind of object is posted into the Qt event queue in order to call some method from the GUI thread
class QmitkCallbackEvent : public QEvent
{

  public:

    QmitkCallbackEvent( itk::Command* cmd )
    : QEvent(QEvent::User),
      m_Command(cmd)
    {
    }

    itk::Command* command()
    {
      return m_Command;
    }

protected:

    itk::Command::Pointer m_Command;
};


QmitkCallbackFromGUIThread::QmitkCallbackFromGUIThread()
{
  mitk::CallbackFromGUIThread::RegisterImplementation(this);
}

QmitkCallbackFromGUIThread::~QmitkCallbackFromGUIThread()
{
}

void QmitkCallbackFromGUIThread::CallThisFromGUIThread(itk::Command* cmd)
{
  std::cout << "posting callback event" << cmd << std::endl;
  qApp->postEvent( this, new QmitkCallbackEvent(cmd) );
}

bool QmitkCallbackFromGUIThread::event( QEvent* e ) 
{
  QmitkCallbackEvent* event( dynamic_cast<QmitkCallbackEvent*>(e) );

  if (!event) return false;

  itk::Command* cmd( event->command() );
  
  std::cout << "callback event" << cmd << std::endl;


  if (cmd)
  {
    const itk::NoEvent dummyEvent;
    cmd->Execute( (const itk::Object*) NULL, // no itk::Object here
                  dummyEvent );
  }
  
  return true;
}

