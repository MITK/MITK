#ifndef QMITK_EVENT_COLLECTOR_H
#define QMITK_EVENT_COLLECTOR_H

#include <qobject.h>
#include <qstring.h>
#include <string>

//#include <Applications/MITKSampleApp/QcMITKSamplePlugin.h>



/** Collects all kind of QT Events.
 * To collect events like like Actions to include in your application use.
 * #include <qapplication.h>
 * qmitk::EventCollector* logger = new qmitk::EventCollector();
 * qApp->installEventFilter(logger);
 * PROGRESS in work!
 */



class EventCollector : public QObject
{
  Q_OBJECT

  public:
    EventCollector( QObject * parent = 0, const char * name = 0 );
    virtual ~EventCollector();


  protected:
    virtual bool eventFilter( QObject *o, QEvent *e );
    void SetMousePressed(bool b);
    bool IsMousePressed();
    void SetKeyDown(bool b);
    bool IsKeyDown();
    QString GetObjectValue(QObject* o, QObject* o2);
    QString Object2String( QEvent::Type event, QObject* o);
    void StringChangeObject( const QString &s );
    void SetEventObject(QObject * o);
    QObject * GetEventObject();
  private:
    bool m_MousePressed;
    bool m_KeyDown;
    QObject* m_EventObject;

};

#endif

