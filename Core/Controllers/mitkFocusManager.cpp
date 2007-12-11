/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
bool mitk::FocusManager::AddElement(FocusElement* element)
{
  // Try find  
  mitk::FocusManager::FocusListIterator position = std::find(m_FocusList.begin(),m_FocusList.end(),element);
	if (position != m_FocusList.end())
		return false;
	
  m_FocusList.push_back(element);
  if (m_FocElement.GetPointer() == NULL)
    m_FocElement = element;
	return true;
}

//##ModelId=3EDCAF790230
bool mitk::FocusManager::RemoveElement(FocusElement* element)
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
mitk::FocusManager::FocusElement* mitk::FocusManager::GetFocused() const 
{
  return m_FocElement.GetPointer();
}

//##ModelId=3EDCAF7902FB
bool mitk::FocusManager::SetFocused(FocusElement* element)
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
const mitk::FocusManager::FocusElement* mitk::FocusManager::GetFirst() const
{
	return (m_FocusList.front()).GetPointer();
}

//##ModelId=3EDCAF7A002D
const mitk::FocusManager::FocusElement* mitk::FocusManager::GetLast() const
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
