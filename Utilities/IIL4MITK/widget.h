#ifndef IIL_WIDGET_WRAPPER
#define IIL_WIDGET_WRAPPER

#include <qgl.h>
#include <assert.h>
#include <qptrlist.h> 

class iilItem;

class iilWidget : public QGLWidget 
{
public:
    iilWidget(QGLFormat glf, QWidget *parent = 0, const char *name = 0) : QGLWidget(glf, parent, name) {};
    iilWidget(QWidget *parent = 0, const char *name = 0) : QGLWidget(parent, name) {};
    ~iilWidget()
    {
    }
    //##Documentation
    //## TEMPORARY FOR IIL COMPATIBILITY - DO NOT USE!!!
  virtual iilWidget* sharedWidget() const {return NULL;}
    //##Documentation
    //## TEMPORARY FOR IIL COMPATIBILITY - DO NOT USE!!!
  virtual bool isVisible (const iilItem* const item) {return true;}
  virtual bool isVisible (const float, const float,
  			  const float, const float) {return true;}
};

typedef iilWidget RenderWindowBase;

namespace mitk {
    
typedef ::QMouseEvent MouseEvent;
class KeyEvent : public ::QKeyEvent
{

    int m_X;
    int m_Y;
    int m_GlobalX;
    int m_GlobalY;
public:
    KeyEvent ( Type type, int key, int ascii, int state, const QString & text = QString::null, bool autorep = FALSE, ushort count = 1, int x=0, int y=0, int globalX=0, int globalY=0 ) :
        ::QKeyEvent(type, key, ascii, state, text, autorep, count), m_X(x), m_Y(y), m_GlobalX(globalX), m_GlobalY(globalY) 
    {
    };
    ~KeyEvent()
    {
    }
    int x () const
    { 
        return m_X;
    }
    int y () const
    { 
        return m_Y;
    }
    int globalX () const
    { 
        return m_GlobalX;
    }
    int globalY () const
    { 
        return m_GlobalY;
    }
};

#define mitkEnum Qt
}
#endif
