#include "PositionEvent.h"

//##ModelId=3E5B7CF900F0
mitk::PositionEvent::PositionEvent(int type, int buttonNumber, int key, const Vector3f& position)
: m_Position(position), Event(type, buttonNumber, key)
{}

//##ModelId=3E5B79DE0133
Vector3f mitk::PositionEvent::GetPosition() const
{
	return m_Position;
}


