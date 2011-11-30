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


#include "mitkSmartPointerProperty.h"

mitk::SmartPointerProperty::ReferenceCountMapType          mitk::SmartPointerProperty::m_ReferenceCount;
mitk::SmartPointerProperty::ReferencesUIDMapType           mitk::SmartPointerProperty::m_ReferencesUID;
mitk::SmartPointerProperty::ReadInSmartPointersMapType     mitk::SmartPointerProperty::m_ReadInInstances;
mitk::SmartPointerProperty::ReadInTargetsMapType           mitk::SmartPointerProperty::m_ReadInTargets;
mitk::UIDGenerator                                         mitk::SmartPointerProperty::m_UIDGenerator("POINTER_");

void mitk::SmartPointerProperty::PostProcessXMLReading()
{
  for (ReadInSmartPointersMapType::iterator iter = m_ReadInInstances.begin();
       iter != m_ReadInInstances.end();
       ++iter)
  {
    if ( m_ReadInTargets.find(iter->second) != m_ReadInTargets.end() )
    {
      iter->first->SetSmartPointer(  m_ReadInTargets[ iter->second ] );
    }
  }

  m_ReadInInstances.clear();
}

/// \return The number of SmartPointerProperties that point to @param object
unsigned int mitk::SmartPointerProperty::GetReferenceCountFor(itk::Object* object)
{
  if ( m_ReferenceCount.find(object) != m_ReferenceCount.end() )
  {
    return m_ReferenceCount[object];
  }
  else
  {
    return 0;
  }
}

void mitk::SmartPointerProperty::RegisterPointerTarget(itk::Object* object, const std::string uid)
{
  m_ReadInTargets[uid] = object;
}

std::string mitk::SmartPointerProperty::GetReferenceUIDFor(itk::Object* object)
{
  if ( m_ReferencesUID.find(object) != m_ReferencesUID.end() )
  {
    return m_ReferencesUID[object];
  }
  else
  {
    return std::string("invalid");
  }
}

bool mitk::SmartPointerProperty::IsEqual(const BaseProperty& property) const
{
  return this->m_SmartPointer == static_cast<const Self&>(property).m_SmartPointer;
}

bool mitk::SmartPointerProperty::Assign(const BaseProperty& property)
{
  this->m_SmartPointer = static_cast<const Self&>(property).m_SmartPointer;
  return true;
}

mitk::SmartPointerProperty::SmartPointerProperty(itk::Object* pointer)
{
  SetSmartPointer( pointer );
}

itk::Object::Pointer mitk::SmartPointerProperty::GetSmartPointer() const
{
    return m_SmartPointer;
}

itk::Object::Pointer mitk::SmartPointerProperty::GetValue() const
{
  return this->GetSmartPointer();
}

void mitk::SmartPointerProperty::SetSmartPointer(itk::Object* pointer)
{
  if(m_SmartPointer.GetPointer() != pointer)
  {
    // keep track of referenced objects
    if ( m_SmartPointer.GetPointer() && --m_ReferenceCount[m_SmartPointer.GetPointer()] == 0 ) // if there is no reference left, delete entry
    {
      m_ReferenceCount.erase( m_SmartPointer.GetPointer() );
      m_ReferencesUID.erase( m_SmartPointer.GetPointer() );
    }

    if ( pointer && ++m_ReferenceCount[pointer] == 1 ) // first reference --> generate UID
    {
      m_ReferencesUID[pointer] = m_UIDGenerator.GetUID();
    }
    
    // change pointer
    m_SmartPointer = pointer;
    Modified();
  }
}

void mitk::SmartPointerProperty::SetValue(const ValueType & value)
{
  this->SetSmartPointer(value.GetPointer());
}

std::string mitk::SmartPointerProperty::GetValueAsString() const
{
  if ( m_SmartPointer.IsNotNull() )
    return m_ReferencesUID[ m_SmartPointer.GetPointer() ];
  else
    return std::string("NULL");
}
