#include "Event.h"

//##ModelId=3E5B3007000F
mitk::Event::Event(int type, int button, int buttonState, int key)
: m_Type(type), m_Button(button), m_ButtonState(buttonState), m_Key(key)  
{}

//##ModelId=3E5B304700A7
int mitk::Event::GetType() const
{
	return m_Type;
}

//##ModelId=3E5B3055015C
int mitk::Event::GetButtonState() const
{
	return m_ButtonState;
}

//##ModelId=3E8AE2F20360
int mitk::Event::GetButton() const
{
	return m_Button;
}


//##ModelId=3E5B306F0221
int mitk::Event::GetKey() const
{
	return m_Key;
}

//##ModelId=3E77630102A1
bool mitk::Event::operator==(const Event& event)
{
	if ( (m_Type == event.GetType())
		&& (m_Button == event.GetButton())
		&& (m_ButtonState == event.GetButtonState())
		&& (m_Key == event.GetKey()) )
		return true;
	else return false;
}
