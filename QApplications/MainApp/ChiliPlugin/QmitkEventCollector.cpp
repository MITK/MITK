/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <QmitkEventCollector.h>
#include <qevent.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <iostream>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qstringlist.h>
#include <qrangecontrol.h>
#include <qvaluelist.h>
#include <qpoint.h>
#include <qobjectlist.h>
#include <qlistview.h>
#include <qwidget.h>
#include <qwidgetlist.h>
#include <mitkCommon.h>
#include <QmitkSliderLevelWindowWidget.h>
#include <mitkLevelWindow.h>

#include <mitkConferenceToken.h>
#include <mitkConferenceKit.h>
#include <stdlib.h>


EventCollector::EventCollector( QObject * parent, const char * name)
    : QObject(parent, name), m_EventObject(NULL),m_Event(NULL) {};
EventCollector::~EventCollector() {};


bool EventCollector::m_PostedEvent = false;
bool EventCollector::m_ReadyToSend = false;
QString EventCollector::m_EventStr = "";
QEvent::Type EventCollector::m_EventID =QEvent::MouseMove ;

const QString Seperator = QString("&&");


bool
EventCollector::eventFilter( QObject *o, QEvent *e )
{
  if(m_EventObject == NULL ) return false;

  mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();

  if(m_ReadyToSend)
  {
    if ( !QString("QGLWidget").compare( QString(m_EventObject->className()) ) )
      if( m_EventID != QEvent::Wheel )
      {
        //std::cout<<"QmitkEventCollector::eventFilter() im MITK Widget und deshalb nicht weiter interessant"<<std::endl;
        m_ReadyToSend = false;
        return false;
      }

    QString complete = m_EventStr + Seperator;   // field 0 - 3 
    complete += GetObjectValue( m_EventObject ) + Seperator; // field 4 - 5
    complete += Object2String( m_EventID, m_EventObject ); //field 6 - ??
    //std::cout<<"QmitkEventCollector::eventFilter(): JETZT WIRD GESENDET: "<<complete.latin1()<<std::endl;

    mitk::ConferenceKit* ck = mitk::ConferenceKit::GetInstance();
    if( ck != NULL )
    {
      ck->SendQt( complete.latin1() );
    }

    m_ReadyToSend = false;
  }

  //only for this events will be handled here other wise return to event handler
  if( e->type() != QEvent::MouseButtonDblClick &&
      e->type() != QEvent::MouseButtonPress &&
      e->type() != QEvent::MouseButtonRelease &&
      e->type() != QEvent::KeyPress &&
      e->type() != QEvent::KeyRelease &&
      e->type() != QEvent::Wheel
      ) return false;

  //Events sended from PostEvent() will not be handled here.
  if(m_PostedEvent)
  {
    m_PostedEvent = false;
    return false;
  }

  // Spontaneous events are created by the OS
  if( !e->spontaneous() ) return false;


  if( !ct->HaveToken() )
  {
      //std::cout<<"QmitkEventCollector::eventFilter() try getting token"<<std::endl;
      ct->GetToken();
  }


  // Neccesary because some internal QT-Objects are named equal.
  // so there is the posibility to use the hirachy for some hidden
  // objects. in this case the hirachie will be shifted here.
  // QViewPortWidget<-QListBox<-QComboBox
  // The QComboBox is the only object with a epxlicitly set name
  if ( !QString(o->className()).compare("QViewportWidget") ||
       !QString(o->name()).compare("in-combo") ||
       ( !QString(o->name()).compare("controls") && !QString(o->className()).compare("QSpinWidget")) 
     )
  {
    m_EventObject = o->parent();
    //std::cout<<"QmitkEventCollector::eventFilter() EVENT OBJECT set to parent..."<<m_EventObject->name()<<std::endl;
  }
  else
  {
    m_EventObject = o;
  }
  m_EventStr    = Event2String( e );
  m_EventID     = e->type();
  m_ReadyToSend = true;

  return false;
}

bool
EventCollector::SetObjectValue( QObject* o, QString className, QString value, QString /*value2*/ )
{

  if (!value.compare("NULL")) return true;

  if(!className.compare("QLineEdit"))
  {
    // QLineEdit
    QLineEdit *qle = dynamic_cast<QLineEdit *>(o);
    if ( qle )
      qle->setText(value);
  }
//  else if( !className.compare( "QListView" ) )
//  {
//    QListView* qlv = dynamic_cast<QListView *>(o);
//    if ( qlv )
//    {
//      qlv->setCurrentItem( qlv->findItem(QString(""),value2.toInt(),ExactMatch) );
//      qlv->setSelected ( qlv->findItem(QString(""),value2.toInt()), true );
//    }
//  }
  else if ( value.isEmpty() )
  {
    return true;
  }

  //std::cout<<"QmitkEventCollector::SetObjectValue(): "<<value.latin1()<<std::endl;

  //QSlider
  if(!className.compare("QSlider"))
  {
    QSlider *qs = dynamic_cast<QSlider *>(o);
    if ( qs )
      qs->setValue(value.toInt());
  }
  else if(!className.compare("QSpinBox") )
  {
    //QSpinBox
    QSpinBox *qsb = dynamic_cast<QSpinBox *>(o);
    if ( qsb )
      qsb->setValue(value.toInt());
     // std::cout<<"EventCollector::SetObjectValue(): QSpinBox oder QSpinWidget = "<<value.latin1()<<std::endl;
  }
  else if(!className.compare("QListBox"))
  {
    //QListBox
    QListBox *qlb =dynamic_cast<QListBox *>(o);
    if ( qlb )
      qlb->setSelected(value.toInt(),true);
  }
  else if(!className.compare("QCheckBox"))
  {
    // QCheckBox
    QCheckBox *qcb = dynamic_cast<QCheckBox *>(o);
    if ( qcb )
    {
      bool v = (value.toInt() == 1)?true:false;
      if(v == qcb->isChecked() ) return false;
    }
  }
//  else if( className.find("Button",0) > 0 )
//  {
//  }
  else if ( !className.compare("QTabBar") )
  {
    QTabBar* qtb = dynamic_cast<QTabBar *>(o);
    if ( qtb )
    {
      qtb->setCurrentTab( value.toInt() );
    }
  }
  else if( !className.compare("QComboBox") )
  {
    QComboBox* qcb = dynamic_cast<QComboBox *>(o);
    if ( qcb )
      qcb->setCurrentItem(value.toInt());
  }
  /* Bug# 135
  else if( !className.compare("QmitkSliderLevelWindowWidget") )
  {
    QmitkSliderLevelWindowWidget* qslw = dynamic_cast<QmitkSliderLevelWindowWidget *>(o);
    if ( qslw )
    {
      mitk::LevelWindow lw = qslw->getLevelWindow();
      lw.SetMinMax(value.toFloat(), value2.toFloat());
      qslw->setLevelWindow(lw);
      qslw->update();
    }
  }*/

  return true;
}

QString
EventCollector::GetObjectValue( QObject * o2 )
{
  QString objvalue;
  QString className = o2->className();

  //QSlider
  if(!className.compare("QSlider"))
  {
    QSlider *qs = dynamic_cast<QSlider *>(o2);
    if( qs )
      objvalue = QString("%1").arg(qs->value());
    objvalue += Seperator;
  }
  else if(!className.compare("QSpinBox") )
  {
    //QSpinBox
    QSpinBox *qsb = dynamic_cast<QSpinBox *>(o2);
    if ( qsb )
      objvalue = QString("%1").arg(qsb->value());
    objvalue += Seperator;
  }
  else if(!className.compare("QListBox"))
  {
    //QListBox
    QListBox *qlb =dynamic_cast<QListBox *>(o2);
    if (qlb )
      objvalue = QString("%1").arg(qlb->currentItem());
    objvalue += Seperator;
  }
  else if(!className.compare("QLineEdit"))
  {
    // QLineEdit
    QLineEdit *qle = dynamic_cast<QLineEdit *>(o2);
    if ( qle )
      objvalue = qle->text();
    objvalue += Seperator;
  }
  else if(!className.compare("QCheckBox"))
  {
    // QCheckBox
    QCheckBox *qcb = dynamic_cast<QCheckBox *>(o2);
    if ( qcb )
      objvalue = (qcb->isChecked())?'1':'0';
    objvalue += Seperator;
  }
  else if( className.find("Button",0) > 0 )
  {
    // for all *Button
    QButton *qb = dynamic_cast<QButton *>(o2);
    if ( qb )
      objvalue = QString("%1").arg((int)qb->state());
    objvalue += Seperator;
    }
  else if ( !className.compare("QTabBar") )
  {
    QTabBar* qtb = dynamic_cast<QTabBar *>(o2);
    if ( qtb )
      objvalue = QString("%1").arg(qtb->currentTab());
    objvalue += Seperator;
  }
  else if( !className.compare("QComboBox"))
  {
    QComboBox* qcb = dynamic_cast<QComboBox *>(o2);
    if ( qcb )
      objvalue = QString("%1").arg(qcb->currentItem());
    objvalue += Seperator;
  }
  else if( !className.compare("QListBox") )
  {
    QListBox* qlb = dynamic_cast<QListBox *>(o2);
    if ( qlb )
      objvalue = QString("%1").arg(qlb->currentItem());
    objvalue += Seperator;
  }
  /* Bug# 135
  else if( !className.compare("QmitkSliderLevelWindowWidget") )
  {
    QmitkSliderLevelWindowWidget* qlb = dynamic_cast<QmitkSliderLevelWindowWidget *>(o2);
    if ( qlb )
    {
      mitk::LevelWindow lw = qlb->getLevelWindow();
      objvalue = QString("%1").arg(lw.GetMin());
      objvalue += Seperator;
      objvalue += QString("%1").arg(lw.GetMax());
    }
    std::cout<<"QmitkSliderLevelWindowWidget Value: "<<objvalue.latin1()<<std::endl;
  }*/


//  else if( !className.compare("QListView") )
//  {
//    QListView* qlv = dynamic_cast<QListView *>(o2);
//    if ( qlv )
//    {
//      QListViewItem* qlvi = qlv->selectedItem();
//      objvalue  = QString("%1").arg( qlvi->text( qlv->itemPos( qlvi ) ) );
//      objvalue += Seperator;
//      objvalue += QString("%1").arg( qlv->itemPos( qlv->selectedItem() ) );
//    }
//  }

  return (objvalue.isNull())?QString("NULL" + Seperator ):objvalue;
}


QString
EventCollector::Object2String( QEvent::Type /*eventid*/, QObject* o)
{
  QStringList objlist;

  //Parent information
  QObject *po = o->parent();
  if( po )
  {
    //GrandParent
    QObject* gpo = po->parent();
    if(gpo)
    {
      objlist += gpo->className();
      objlist += gpo->name();
    }
    objlist += po->className();
    objlist += po->name();
  }
  
  objlist += o->className();
  objlist += o->name();


  return objlist.join(Seperator);
}

QString
EventCollector::Event2String(QEvent* event)
{
  QStringList objlist;

  // 1. Event Type
  objlist = QString("%1").arg(event->type());

   // if MouseEvent
   if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease )
   {
     // 2. M Button
     int button_ = static_cast<QMouseEvent *>(event)->button();
     objlist += QString("%1").arg(button_);
     // 3. M State
     int state_ =static_cast<QMouseEvent *>(event)->state();
     objlist += QString("%1").arg(state_);
     //4. empty field
     objlist += "";
   }
   else if ( event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease )
   {
     // 2. Key
     int key_ = static_cast<QKeyEvent *>(event)->key();
     objlist += QString("%1").arg(key_);
     // 3. ASCII
     int ascii_ = static_cast<QKeyEvent *>(event)->ascii();
     objlist += QString("%1").arg(ascii_);
     // 4. State
     int state_ = static_cast<QKeyEvent *>(event)->state();
     objlist += QString("%1").arg(state_);

   }
   else if ( event->type() == QEvent::Wheel )
   {
     // 2. Wheel delta rotation
     int delta_ = static_cast<QWheelEvent *>(event)->delta();
     objlist += QString("%1").arg(delta_);
     // 3. Wheel orientation
     int orientation_ = static_cast<QWheelEvent *>(event)->orientation();
     objlist += QString("%1").arg(orientation_);
     // 4. Wheel State
     int state_ = static_cast<QWheelEvent *>(event)->state();
     objlist += QString("%1").arg(state_);
   }

  return objlist.join(Seperator);
};


bool
EventCollector::PostEvent(QString str, QWidget* w)
{
  QObject* aObject = NULL;
  
  //encrypt string
  QStringList objlist;
  objlist = objlist.split ( Seperator , str, true );


  //QEvent::Type event_t = (QEvent::Type )( *it).toInt();
  QEvent::Type event_t  = (QEvent::Type )(*objlist.at(0)).toInt();
  int event_v1          = (*objlist.at(1)).toInt();
  int event_v2          = (*objlist.at(2)).toInt();
  int event_v3          = (*objlist.at(3)).toInt();
  
  QString value              = (*objlist.at(4));
  QString value2             = (*objlist.at(5));

  
  QStringList::Iterator it = objlist.begin();
  for(int i=0; i<6; i++)
    ++it;
 
  // in dependence to the number of hierachie values sended this 
  // loop will try to follow the structure of the Qt-object tree.
  // if this for reasans like naming is not possible this loop will try
  // to find an child of the allready configuered objects.
  // NOTE: this is importent because uses fixed names for some sub objects.
  //       only following the tree can identify the object of the desired event.
  QString oName ;
  QString oClass;
  while( objlist.end() != it++ && objlist.end()!=it++ )
  {
    
    oName  = *(--it);
    oClass = *(--it);
    ++it; 
    ++it;
    
    if( aObject == NULL )
    {
      QObjectList *pl = w->queryList( oClass, oName  );
      QObjectListIt p_iter( *pl );

      if ( p_iter.count() == 1 )
      {
        aObject = p_iter.current();
        //std::cout<<"aObject war NULL jetzt: "<<oClass<<" --> "<< oName <<std::endl;
      }
      else
      {
        //std::cout<<"aObject BLEIBT NULL (iterator): "<<oClass<<" --> "<< oName <<std::endl;
      }
      delete pl;
    }
    else
    {
      QObjectList *pl = aObject->queryList( oClass, oName  );
      QObjectListIt p_iter( *pl );
      
      if ( p_iter.count() == 1 )
      {
        aObject = p_iter.current();
        //std::cout<<"aObject hat ein Kind: "<<oClass<<" --> "<< oName <<std::endl;
      }
      else
      {
        QObjectList *pl1 = w->queryList( oClass, oName  );
        QObjectListIt p_iter( *pl1 );
      
        if ( p_iter.count() == 1 )
        {
          aObject = p_iter.current();
          //std::cout<<"aObject hatte KEIN Kind aber eindeutige Klasse vom Sample Widget: "<<oClass<<" --> "<< oName <<std::endl;
        }
        else
        {       
          //last try child from last aObject
          if( p_iter.count()>1 )            
          {
            // if there is no child with this characterisation in last instance there will be only NULL in the Object
            aObject = aObject->child( oName, oClass, false );
            //std::cout<<"aObject last TRY: "<<oClass<<" --> "<< oName <<std::endl;
          }
          else
          {
            std::cout<<"DIESEN TYP GIBT ES IN DER GANZEN APP NICHT: "<<oClass<<" --> "<< oName <<std::endl;
            aObject = NULL;
          }
        }
        delete pl1;
      }
      delete pl;    
    }
  }
  if( !aObject )
  {
    //last possibilitie for stuff from outside widgets if there is the following keyword we will search all widgets.
    if( oName.find("TeleConference123",0) > 0 )
    {
      // Show all hidden top level widgets.
      QWidgetList  *list = qApp->topLevelWidgets();
      QWidgetListIt w_it( *list );  // iterate over the widgets
      QWidget * wExt;
      while ( (wExt=w_it.current()) != 0 ) {   // for each top level widget...
        ++w_it;
        QObjectList *l = wExt->queryList( oClass, oName  );
        QObjectListIt p_iter( *l );
//        std::cout<<"EXTERN: "<<oClass<<" --> "<<oName<<" ( "<<p_iter.count()<<" )"<<wExt->name()<<std::endl;
        if ( p_iter.count() == 1 )
        {
          aObject = p_iter.current();
          delete l;
          break;
        }
        delete l;
         
      }
      delete list;                // delete the list, not the widgets
    }
  }
  
  bool send = false;
  if(aObject)
  {
    std::cout<<"POST: "<<aObject->className()<<" --> "<< aObject->name()<<std::endl;
    // use global exception for wheel events send. mitkEventMapper will use the not handle wheel events!
    if( aObject->inherits( "QGLWidget" ) && event_t!=QEvent::Wheel )
    {
      //std::cout<<"QmitkEventCollector::PostEvent() WEG mit MITK WIDGET EVENTS"<<std::endl;
      return false;
    }

    if( event_t == QEvent::MouseButtonRelease || event_t == QEvent::KeyRelease || event_t == QEvent::MouseButtonDblClick )
    {
      if( !SetObjectValue( aObject, QString(aObject->className()), value, value2 ))
      {
        //std::cout<<"QmitkEventCollector::PostEvent() THIS EVENT has no effect..."<<std::endl;
        return false;
      }
    }

    if(event_t == QEvent::MouseButtonRelease || event_t == QEvent::MouseButtonPress || event_t == QEvent::MouseButtonDblClick )
    {
      Qt::ButtonState bsButton = (Qt::ButtonState) event_v1;
      Qt::ButtonState bsState  = (Qt::ButtonState) event_v2;

      QMouseEvent* me = new QMouseEvent( event_t, QPoint(2,2), bsButton, bsState);

      //std::cout<<"QmitkEventCollector::PostEvent() _mouse_ TRY POSTING TO qApp"<<std::endl;
      qApp->postEvent(aObject, me);
      m_PostedEvent = true;
      send = true;
    }
    else if ( event_t == QEvent::KeyPress || event_t == QEvent::KeyRelease )
    {
       int key   =  event_v1;
       int ascii =  event_v2;
       Qt::ButtonState bsState =  (Qt::ButtonState) event_v3;

       QKeyEvent* ke = new   QKeyEvent(event_t, ascii, key, bsState );

       //std::cout<<"QmitkEventCollector::PostEvent() _key_ TRY POSTING TO qApp"<<std::endl;
       qApp->postEvent(aObject, ke);
       m_PostedEvent = true;
       send = true;
    }
    else if( event_t == QEvent::Wheel )
    {
       int delta_ =  event_v1;
       int state_  =  event_v2;
       Qt::Orientation orientation_ =(Qt::Orientation) event_v3;

       QWheelEvent* turn = new QWheelEvent ( QPoint(10,10), delta_, state_, orientation_ );

       qApp->postEvent( aObject, turn );
       m_PostedEvent = true;
       send = true;
    }
  }
  else
  {
    std::cout<<" EVENT Konnte nicht zugeordnet werden - "<<std::endl;
    send = false;
  }

  return send;
}

