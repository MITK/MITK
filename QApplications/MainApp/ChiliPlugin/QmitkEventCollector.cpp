#include "QmitkEventCollector.h"
#include <qevent.h>
#include <qobject.h>
#include <iostream>

//#include <fstream.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qstringlist.h>
#include <qrangecontrol.h>
#include <qvaluelist.h>
#include <qapplication.h>
#include <qpoint.h>
#include <qobjectlist.h>

//#include <mitkConferenceToken.h>
//#include <mitkConferenceKitFactory.h>
#include <mitkConferenceKit.h>
#include <stdlib.h>

EventCollector::EventCollector( QObject * parent, const char * name)
    : QObject(parent, name), m_EventObject(NULL),m_KeyDown(false),m_MousePressed(false) {};
EventCollector::~EventCollector() {};


bool EventCollector::eventFilter( QObject *o, QEvent *e )
{
  std::ostream &logFile=std::cout;
  //std::ofstream logFile("/home/hasselberg/chiliTele.ses", std::ios::app);
  //std::ofstream logFile("chiliTele.ses", std::ios::app);


//   mitk::ConferenceToken* cf = new mitk::ConferenceToken;
//   if(!cf->HaveToken())
//   {
//
//     if(!cf->GetToken())
//     {
//       logFile<<"q_TOKEN: "<<"FALSE"<<std::endl;
//       return false;
//     }
//     logFile<<"q_TOKEN: "<<"get it"<<std::endl;
//   }

  if(e->type()==5) return false; //no MouseMove yet

  //create Object for having a look in the event history
  QString value="undefined";
  //Unterscheidung zwischen System Events und selbsterstellten...

  if( this->m_EventObject != NULL )
  {
    //QWidget Value of interesst -QLine, QCheckBox...
    QString complete = Object2String(e->type(), o);
    if( complete.isNull() )
      std::cout<<"1_-_-_NULL"<<std::endl;
    else
    {
       //Teleconfenrence -->send to XXX
       mitk::ConferenceKit* ck = mitk::ConferenceKit::GetInstance();
       ck->SendQt( complete.latin1() );
     // //StringChangeObject(complete);
    }

    this->SetEventObject( NULL );
  }

  switch( e->type() )
  {
    case QEvent::MouseButtonPress: logFile << "MouseButtonPress"<<std::endl; SetMousePressed(true); break;
    case QEvent::KeyPress: logFile << "KeyPress"<<std::endl; SetKeyDown(true); break;
    //case QEvent::MouseMove:logFile << "MouseMove"; break;
    case QEvent::KeyRelease: logFile << "KeyRelease";
    case QEvent::MouseButtonRelease: logFile << "MouseButtonRelease ->";
                if(IsMousePressed() || IsKeyDown())
                {
                  this->SetEventObject(o);
                  logFile << " Object Gesetzt gespeichert!";
                }
                SetMousePressed(false);
                SetKeyDown(false);
                logFile<<std::endl;
                break;

    //case QEvent::MouseButtonDblClick: logFile << "MouseButtonDblClick"; break;
    //case QEvent::Wheel: logFile<< "MouseWhele"; break;
    //case QEvent::Accel: logFile<< "Wahrscheinlich Shortcut"; break;
    default: ;//logFile<<std::endl;
  }

  //logFile<<std::endl;

  return false;
}

QString EventCollector::GetObjectValue( QObject * o, QObject * o2 )
{
  QString objvalue="";
  QString className = o2->className();

  //QSlider
  if(!className.compare("QSlider"))
  {
    QSlider *qs = static_cast<QSlider *>(o2);
    objvalue.number(qs->value());
  }

  //QSpinBox
  if(!className.compare("QSpinWidget") )
  {
    QSpinWidget *qsb = static_cast<QSpinWidget *>(o2);
    objvalue = (qsb->isDownEnabled())?"-1":"1";
  }

  //QListBox
  if(!className.compare("QListBox"))
  {
    QListBox *qlb =static_cast<QListBox *>(o2);
    objvalue = objvalue.number(qlb->currentItem());
  }

  // QLineEdit
  if(!className.compare("QLineEdit"))
  {
    QLineEdit *qle = static_cast<QLineEdit *>(o2);
    objvalue = qle->text();
  }

  // QCheckBox
  if(!className.compare("QCheckBox"))
  {
    QCheckBox *qcb = static_cast<QCheckBox *>(o2);
    objvalue = (qcb->isChecked())?'1':'0';
  }

  // for all *Button
  QString button = className.right(6);
  //std::cout<<button<<className<<std::endl;
  if(!button.compare("Button"))
  {
    QButton *qb = static_cast<QButton *>(o);
    QButton::ToggleState state = qb->state();
    if(state==QButton::On) objvalue='2';
    else if(state==QButton::NoChange) objvalue='1';
    else if(state==QButton::Off) objvalue='0';

    }

  return objvalue;
}

void EventCollector::SetKeyDown(bool b) { this->m_KeyDown = b; };

bool EventCollector::IsKeyDown() {return this->m_KeyDown; };

void EventCollector::SetMousePressed(bool b) { this->m_MousePressed = b; };

bool EventCollector::IsMousePressed() {return this->m_MousePressed; };

void EventCollector::SetEventObject(QObject * o){  this->m_EventObject = o; };

QObject* EventCollector::GetEventObject(){ return this->m_EventObject; };

QString EventCollector::Object2String( QEvent::Type event, QObject* o)
{
  const QString sep = QString("&&");
  QStringList objlist;
  QString seventid;
  seventid = seventid.setNum(event);
  objlist += seventid;
  objlist += o->className();
  objlist += o->name();

   QObject * o2 = new QObject;
   o2=this->GetEventObject();
   if(o2==NULL) return NULL;
   QString value = GetObjectValue(o, o2);
   //if(value.isNull()) return NULL;
   if(value.isEmpty()) return NULL;
   objlist += value;

  return objlist.join(sep);
};

void EventCollector::StringChangeObject( const QString &s )
{
  QStringList objlist;
  objlist = objlist.split ( QString("&&") , s, true );
  std::cout<<s.latin1()<<std::endl;

  QStringList::Iterator it = objlist.begin();

  QEvent::Type event_t = (QEvent::Type )( *it).toInt();
  QString inheritclass = (*objlist.at(1));
  QString objectname   = (*objlist.at(2));
  QString value        = (*objlist.at(3));


  QObjectList *l = qApp->queryList( inheritclass, objectname );
  QObjectListIterator iter( *l ); // iterate over all objects
  QObject *obj;

  if(obj = iter.current())
    std::cout << "Erfolg: " << obj->name() << iter.count() << std::endl;
  //while ( (obj = it.current()) != 0 ) {
  //  // for each found object...
  //  ++it;
  //  ((QButton*)obj)->setEnabled( FALSE );
  //}
  delete l; // delete the list, not the objects

  //QString object = (*objlist.at(1)).latin1();
  //for ( QStringList::Iterator it = objlist.begin(); it != objlist.end(); ++it )
  //{
  //  std::cout<<(*it).latin1()<<" : ";
  //}
  //std::cout << std::endl;
  //QPoint pos(1,1);
  //QMouseEvent me( QEvent::MouseButtonPress, pos, 0, 0 );
  /*if(QApplication::sendEvent( , &me ))
    std::cout << "Event return bool" << std::endl;
  else
    std::cout << "Event return false" << std::endl;
*/
};


