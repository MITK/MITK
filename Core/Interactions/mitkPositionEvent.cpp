#include "PositionEvent.h"
#include "mitkVector.h"

//##ModelId=3E5B7CF900F0
mitk::PositionEvent::PositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition, const mitk::Point3D& worldPosition)
: DisplayPositionEvent(sender, type, button, buttonState, key, displPosition), m_WorldPosition(worldPosition)
{}

//##ModelId=3EDC8EEC00F8
const mitk::Point3D& mitk::PositionEvent::GetWorldPosition() const
{
	return m_WorldPosition;
}

//##ModelId=3EDC8EEC0136
void mitk::PositionEvent::SetWorldPosition(const mitk::Point3D& worldPosition)
{
	m_WorldPosition = worldPosition;
}
