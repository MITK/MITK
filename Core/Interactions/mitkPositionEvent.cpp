#include "PositionEvent.h"
#include "mitkVector.h"

//##ModelId=3E5B7CF900F0
mitk::PositionEvent::PositionEvent(int type, int button, int buttonState, int key, const mitk::Point3D& displPosition)
: Event(type, button, buttonState, key), m_DisplayPosition(displPosition)
{}

//##ModelId=3E5B79DE0133
mitk::Point3D mitk::PositionEvent::GetDisplayPosition() const
{
	return m_DisplayPosition;
}

//##ModelId=3EDC8EEC00F8
mitk::Point3D mitk::PositionEvent::GetWorldPosition() const
{
	return m_WorldPosition;
}

//##ModelId=3EDC8EEC0136
void mitk::PositionEvent::SetWorldPosition(mitk::Point3D& worldPosition)
{
	m_WorldPosition = worldPosition;
}



