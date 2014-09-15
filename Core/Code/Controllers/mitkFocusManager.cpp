/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkFocusManager.h"

mitk::FocusManager::FocusManager()
{
  m_Loop = true;//default
  m_FocusList.clear();
  m_FocElement = NULL;
}

mitk::FocusManager::~FocusManager()
{
}

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

bool mitk::FocusManager::RemoveElement(FocusElement* element)
{
  if (element == m_FocElement)
  {
    this->GoToNext();
  }

  // Try to find
  mitk::FocusManager::FocusListIterator position = std::find(m_FocusList.begin(), m_FocusList.end(), element);
  if (position == m_FocusList.end())
  {
    return false;
  }

  m_FocusList.erase(position);
  if (m_FocusList.empty())
  {
    m_FocElement = NULL;
  }

  return true;
}

mitk::FocusManager::FocusElement* mitk::FocusManager::GetFocused() const
{
  return m_FocElement.GetPointer();
}

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

bool mitk::FocusManager::IsLast()
{
  return (m_FocElement == m_FocusList.back());
}

bool mitk::FocusManager::IsFirst()
{
  return (m_FocElement == m_FocusList.front());
}

const mitk::FocusManager::FocusElement* mitk::FocusManager::GetFirst() const
{
  return (m_FocusList.front()).GetPointer();
}

const mitk::FocusManager::FocusElement* mitk::FocusManager::GetLast() const
{
  return (m_FocusList.back()).GetPointer();
}

bool mitk::FocusManager::GoToNext()
{
  if (m_FocusList.empty())
  {
    return false;
  }

  //find the m_FocElement
  FocusListIterator position = std::find(m_FocusList.begin(), m_FocusList.end(), m_FocElement);
  if (position == m_FocusList.end())
  {
    return false;
  }

  if (m_FocusList.size() == 1)
  {
    return true;
  }

  FocusListIterator nextPosition = position + 1;
  while(nextPosition != position)
  {
    if (nextPosition == m_FocusList.end())
    {
      if (!m_Loop)
      {
        return false;
      }
      nextPosition = m_FocusList.begin();
    }

    FocusElement* focusElement = *nextPosition;
    if (focusElement->GetSizeX() > 0 && focusElement->GetSizeY() > 0)
    {
      m_FocElement = focusElement;
      return true;
    }
    ++nextPosition;
  }

  m_FocElement = NULL;
  return false;
}

//##Documentation
//## returns an iterator, that points to the
//## beginning of the list
mitk::FocusManager::FocusListIterator mitk::FocusManager::GetIter()
{
  return m_FocusList.begin();
}

void mitk::FocusManager::SetLoop(bool loop)
{
  m_Loop = loop;
}
