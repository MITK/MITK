#include "mitkFocusManager.h"

//##ModelId=3EDCAF790194
mitk::FocusManager::FocusManager()
{
	m_Loop = true;//default
	m_FocusList.clear();
	m_FocElement = NULL;
}

mitk::FocusManager::~FocusManager()
{
}

//##ModelId=3EDCAF7901C3
bool mitk::FocusManager::AddElement(FocusElement::Pointer element)
{
  // Try find  
  mitk::FocusManager::FocusListIterator position = std::find(m_FocusList.begin(),m_FocusList.end(),element);
	if (position != m_FocusList.end())
		return false;
	
  m_FocusList.push_back(element);
  if (m_FocElement.IsNull())
    m_FocElement = element;
	return true;
}

//##ModelId=3EDCAF790230
bool mitk::FocusManager::RemoveElement(mitk::FocusManager::FocusElement::Pointer element)
{
	// Try find  
	mitk::FocusManager::FocusListIterator position = std::find(m_FocusList.begin(),m_FocusList.end(),element);
	if (position == m_FocusList.end())
		return false;
	position = m_FocusList.erase(position);
	//first delete the one on the position, and store the one afterewards into position
	if (position == m_FocusList.end())//deleded was the last in row, then take the one before
		m_FocElement = m_FocusList.back();
	else
		m_FocElement = *position;//m_FocElement is equal to the next one in row
	return true;
}

//##ModelId=3EDCAF7902BD
mitk::FocusManager::FocusElement::ConstPointer mitk::FocusManager::GetFocused() const 
{
  return m_FocElement.GetPointer();
}

//##ModelId=3EDCAF7902FB
bool mitk::FocusManager::SetFocused(FocusElement::Pointer element)
{
  if (m_FocElement == element)
    return true;
  FocusListIterator position = std::find(m_FocusList.begin(),m_FocusList.end(),element);
	if (position == m_FocusList.end())//not found
		return false;

	m_FocElement = *position;
  ((const itk::Object*)this)->InvokeEvent(FocusEvent());
	return true;
}

//##ModelId=3EDCAF790378
bool mitk::FocusManager::IsLast()
{
	return (m_FocElement == m_FocusList.back());
}

//##ModelId=3EDCAF7903A7
bool mitk::FocusManager::IsFirst()
{
	return (m_FocElement == m_FocusList.front());
}

//##ModelId=3EDCAF7903D6
mitk::FocusManager::FocusElement::ConstPointer mitk::FocusManager::GetFirst() const
{
	return (m_FocusList.front()).GetPointer();
}

//##ModelId=3EDCAF7A002D
mitk::FocusManager::FocusElement::ConstPointer  mitk::FocusManager::GetLast() const
{
	return (m_FocusList.back()).GetPointer();
}

//##ModelId=3EDCAF7A007B
bool mitk::FocusManager::GoToNext()
{
	//find the m_FocElement
	FocusListIterator position = std::find(m_FocusList.begin(),m_FocusList.end(),m_FocElement);
	if (position == m_FocusList.end())//not found
		return false;
	else if (*position == m_FocusList.back())//last in row
	{
		if (m_Loop)
		{
			m_FocElement = *(m_FocusList.begin());
			return true;
		}
		else
		{
			return false;//last in row and loop == false, so GoToNext == false
		}
	}
	else //not last in row
	{
		m_FocElement = *(++position);//increase position and set m_FocElement
		return true;
	}
	return false;
}

//##ModelId=3EDCAF7A00AA
//##Documentation
//## returns an iterator, that points to the 
//## beginning of the list
mitk::FocusManager::FocusListIterator mitk::FocusManager::GetIter() 
{
	return m_FocusList.begin();
}

//##ModelId=3EDCAF7A00D8
void mitk::FocusManager::SetLoop(bool loop)
{
	m_Loop = loop;
}
