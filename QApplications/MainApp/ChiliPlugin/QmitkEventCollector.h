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

#ifndef QMITK_EVENT_COLLECTOR_H
#define QMITK_EVENT_COLLECTOR_H

#include <qobject.h>
#include <qstring.h>
#include <string>



/** Collects all kind of QT events.
 * To collect events like like Actions to include in your application use.
 * #include <qapplication.h>
 * qmitk::EventCollector* logger = new qmitk::EventCollector();
 * qApp->installEventFilter(logger);
 * 
 */

class EventCollector : public QObject
{
  Q_OBJECT

  public:
    EventCollector( QObject * parent = 0, const char * name = 0 );
    virtual ~EventCollector();
    static bool PostEvent(QString, QWidget* w);


  protected:
    QString Event2String( QEvent* event);
    virtual bool eventFilter( QObject *o, QEvent *e );
    void SetMousePressed(bool b);
    bool IsMousePressed();
    void SetKeyDown(bool b);
    bool IsKeyDown();
    QString Object2String( QEvent::Type eid, QObject* o);
    QObject * GetEventObject();

  private:
    QString GetObjectValue( QObject* o2);
    static bool SetObjectValue( QObject * o, QString className, QString value, QString value2="" );
    static bool m_ReadyToSend;
    static bool m_PostedEvent;
    QObject* m_EventObject;
    QEvent*  m_Event;
    static QString m_EventStr;
    static QEvent::Type m_EventID;

};

#endif

