#ifndef MITK_EVENT_TYPEDEFS
#define MITK_EVENT_TYPEDEFS

#include <qgl.h>
#include <assert.h>
#include <qptrlist.h> 

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
