#include "Event.h"

//##ModelId=3E5B3007000F
mitk::Event::Event(int type, int buttonNumber, int key)
: m_Type(type), m_ButtonNumber(buttonNumber), m_Key(key)  
{}

//##ModelId=3E5B304700A7
int mitk::Event::GetType() const
{
	return m_Type;
}

//##ModelId=3E5B3055015C
int mitk::Event::GetButtonNumber() const
{
	return m_ButtonNumber;
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
		&& (m_ButtonNumber == event.GetButtonNumber())
		&& (m_Key == event.GetKey()) )
		return true;
	else return false;
}

