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
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>
    
const char* mitk::SmartPointerProperty::XML_SMARTPOINTER_TARGET_NODE = "smartPointerTarget";
const char* mitk::SmartPointerProperty::XML_SMARTPOINTER_TARGET_KEY = "POINTERTARGET_ID";

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

//##ModelId=3ED94B4203C1
bool mitk::SmartPointerProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_SmartPointer==m_SmartPointer;
}

//##ModelId=3ED94B7500F2
mitk::SmartPointerProperty::SmartPointerProperty(itk::Object* pointer)
{
  SetSmartPointer( pointer );
}

//##ModelId=3ED94B750111
mitk::SmartPointerProperty::~SmartPointerProperty()
{
}

//##ModelId=3ED952AD02B7
itk::Object::Pointer mitk::SmartPointerProperty::GetSmartPointer() const
{
    return m_SmartPointer;
}

//##ModelId=3ED952AD02F6
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

std::string mitk::SmartPointerProperty::GetValueAsString() const
{
  if ( m_SmartPointer.IsNotNull() )
    return m_ReferencesUID[ m_SmartPointer.GetPointer() ];
  else
    return std::string("NULL");
}

bool mitk::SmartPointerProperty::Assignable(const BaseProperty& other) const
{
  try
  {
    dynamic_cast<const Self&>(other); // dear compiler, please don't optimize this away!
    return true;
  }
  catch (std::bad_cast)
  {
  }
  return false;
}

mitk::BaseProperty& mitk::SmartPointerProperty::operator=(const BaseProperty& other)
{
  try
  {
    const Self& otherProp( dynamic_cast<const Self&>(other) );

    if (this->m_SmartPointer != otherProp.m_SmartPointer)
    {
      this->m_SmartPointer = otherProp.m_SmartPointer;
      this->Modified();
    }
  }
  catch (std::bad_cast)
  {
    // nothing to do then
  }

  return *this;
}


bool mitk::SmartPointerProperty::ReadXMLData( XMLReader& xmlReader)
{
  std::string pointedAt;
  xmlReader.GetAttribute( VALUE, pointedAt );

  if ( pointedAt != "NULL" )
  {
    m_ReadInInstances[this] = pointedAt;
  }
  
  return true;
}

