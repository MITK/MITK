#include "mitkDisplayPositionEvent.h"
#include "mitkVector.h"

mitk::DisplayPositionEvent::DisplayPositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition)
: Event(sender, type, button, buttonState, key), m_DisplayPosition(displPosition)
{}

const mitk::Point2D& mitk::DisplayPositionEvent::GetDisplayPosition() const
{
	return m_DisplayPosition;
}


