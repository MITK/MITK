#include "QmitkCallbackFromGUIThread.h"

#include<qapplication.h>

/// This kind of object is posted into the Qt event queue in order to call some method from the GUI thread
class QmitkCallbackEvent : public QEvent
{

  public:

    QmitkCallbackEvent( itk::Command* cmd, itk::EventObject* e )
    : QEvent(QEvent::User),
      m_Command(cmd),
      m_Event(e)
    {
    }

    ~QmitkCallbackEvent()
    {
      delete m_Event;
    }

    itk::Command* command()
    {
      return m_Command;
    }

    itk::EventObject* itkevent()
    {
      return m_Event;
    }

protected:

    itk::Command::Pointer m_Command;
    itk::EventObject* m_Event;
};


QmitkCallbackFromGUIThread::QmitkCallbackFromGUIThread()
{
  mitk::CallbackFromGUIThread::RegisterImplementation(this);
}

QmitkCallbackFromGUIThread::~QmitkCallbackFromGUIThread()
{
}

void QmitkCallbackFromGUIThread::CallThisFromGUIThread(itk::Command* cmd, itk::EventObject* e)
{
  std::cout << "posting callback event" << cmd << std::endl;
  qApp->postEvent( this, new QmitkCallbackEvent(cmd, e) );
}

bool QmitkCallbackFromGUIThread::event( QEvent* e ) 
{
  QmitkCallbackEvent* event( dynamic_cast<QmitkCallbackEvent*>(e) );

  if (!event) return false;

  itk::Command* cmd( event->command() );
  
  std::cout << "callback event" << cmd << std::endl;


  if (cmd)
  {
    std::cout << "executing cmd " << cmd << std::endl;
    if (event->itkevent())
    {
      cmd->Execute( (const itk::Object*) NULL, // no itk::Object here
                    *(event->itkevent()) );
    }
    else
    {
      const itk::NoEvent dummyEvent;
      cmd->Execute( (const itk::Object*) NULL, // no itk::Object here
                    dummyEvent );
    }
  }
  
  return true;
}

