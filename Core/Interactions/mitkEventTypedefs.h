#ifndef MITK_EVENT_TYPEDEFS
#define MITK_EVENT_TYPEDEFS

#include <qgl.h>
#include <assert.h>
#include <qptrlist.h> 
#include <string>
namespace mitk {
    
typedef ::QMouseEvent MouseEvent;
typedef ::QWheelEvent WheelEvent;

class KeyEvent {

    int m_X;
    int m_Y;
    int m_GlobalX;
    int m_GlobalY;
    int m_State;
    int m_Ascii;
    int m_Type;
    int m_Key;
    std::string m_Text;
  public:
    KeyEvent ( int type, int key, int ascii, int state, const std::string & text = "", bool autorep = FALSE, ushort count = 1, int x=0, int y=0, int globalX=0, int globalY=0 )  :
 m_X(x), m_Y(y), m_GlobalX(globalX), m_GlobalY(globalY) , m_State(state),m_Ascii(ascii),m_Text(text),m_Type(type),m_Key(key)
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

    bool isAccepted() const {
      return true;
    }
    int state() const {
      return m_State;
    }
    int ascii() const {
      return m_Ascii;
    }
    const char* text() const {
      return m_Text.c_str();
    }
     int type() const {
      return m_Type;
    }
      int key() const {
      return m_Key;
    }
    
};

#define mitkEnum Qt
}
#endif
