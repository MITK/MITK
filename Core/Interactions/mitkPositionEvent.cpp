#include "PositionEvent.h"

//##ModelId=3E5B7CF900F0
mitk::PositionEvent::PositionEvent(int type, int button, int buttonState, int key, const Vector3f& position)
: m_Position(position), Event(type, button, buttonState, key)
{}

//##ModelId=3E5B79DE0133
Vector3f mitk::PositionEvent::GetPosition() const
{
	return m_Position;
}


