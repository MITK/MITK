#ifndef MITK_EVENT_TYPEDEFS
#define MITK_EVENT_TYPEDEFS

#include <assert.h>
#include <string>
#include "mitkDisplayPositionEvent.h"

namespace mitk {

class WheelEvent;

class BaseEvent 
{
public:
  BaseEvent(int type, int state, int x = 0,int y = 0, int globalX=0, int globalY = 0) : m_X(x), m_Y(y), m_GlobalX(globalX), m_GlobalY(globalY) , m_State(state),m_Type(type) {};
protected:
  int m_X;
  int m_Y;
  int m_GlobalX;
  int m_GlobalY;
  int m_State; 
  int m_Type;
};

typedef DisplayPositionEvent MouseEvent;

class KeyEvent : public BaseEvent 
{
  int m_Ascii;
  std::string m_Text;
  int m_Key;
public:
  KeyEvent(int type, int key, int ascii, int state, const std::string & text = "", bool autorep = false, unsigned short count = 1, int x=0, int y=0, int globalX=0, int globalY=0 )
    : BaseEvent(type,state,x,y,globalX,globalY) ,m_Ascii(ascii),m_Text(text),m_Key(key)
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

}
#endif
