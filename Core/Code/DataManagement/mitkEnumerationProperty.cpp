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

#include "mitkEnumerationProperty.h"
#include <algorithm>

// static map members of EnumerationProperty. These Maps point to per-classname-maps of ID <-> String. Accessed by GetEnumIds() and GetEnumString().
mitk::EnumerationProperty::IdMapForClassNameContainerType     mitk::EnumerationProperty::s_IdMapForClassName;
mitk::EnumerationProperty::StringMapForClassNameContainerType mitk::EnumerationProperty::s_StringMapForClassName;


mitk::EnumerationProperty::EnumerationProperty()
{
  m_CurrentValue = 0;
}


bool mitk::EnumerationProperty::AddEnum( const std::string& name, const IdType& id )
{
  if ( ( ! IsValidEnumerationValue( name ) ) && ( ! IsValidEnumerationValue( id ) ) )
  {
    GetEnumIds().insert( std::make_pair( id, name ) );
    GetEnumStrings().insert( std::make_pair( name, id ) );
    return true;
  }
  else
  {
    return false;
  }
}


bool mitk::EnumerationProperty::SetValue( const std::string& name )
{
  if ( IsValidEnumerationValue( name ) )
  {
    m_CurrentValue = GetEnumId( name );
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::EnumerationProperty::SetValue( const IdType& id )
{
  if ( IsValidEnumerationValue( id ) )
  {
    m_CurrentValue = id;
    return true;
  }
  else
  {
    return false;
  }
}

mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetValueAsId() const
{
  return m_CurrentValue;
}

std::string mitk::EnumerationProperty::GetValueAsString() const
{
  return GetEnumString( m_CurrentValue );
}


void mitk::EnumerationProperty::Clear()
{
  GetEnumIds().clear();
  GetEnumStrings().clear();
  m_CurrentValue = 0;
}


unsigned int mitk::EnumerationProperty::Size() const
{
  return GetEnumIds().size();
}



mitk::EnumerationProperty::EnumConstIterator mitk::EnumerationProperty::Begin() const
{
  return GetEnumIds().begin();
}


mitk::EnumerationProperty::EnumConstIterator mitk::EnumerationProperty::End() const
{
  return GetEnumIds().end();
}


std::string mitk::EnumerationProperty::GetEnumString( const IdType& id ) const
{
  if ( IsValidEnumerationValue( id ) )
  {
    return GetEnumIds().find( id )->second;
  }
  else
  {
    return "invalid enum id or enums empty";
  }
}


mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetEnumId( const std::string& name ) const
{
  if ( IsValidEnumerationValue( name ) )
  {
    return  GetEnumStrings().find( name )->second;
  }
  else
  {
    return 0;
  }
}


bool mitk::EnumerationProperty::operator==( const BaseProperty& property ) const
{
  const Self * other = dynamic_cast<const Self*>( &property );

  if ( other == NULL )
    return false;

  if ( this->Size() != other->Size() )
    return false;

  if ( this->GetValueAsId() != other->GetValueAsId() )
    return false;

  return std::equal( this->Begin(), this->End(), other->Begin() );
}


bool mitk::EnumerationProperty::IsValidEnumerationValue( const IdType& val ) const
{
  return ( GetEnumIds().find( val ) != GetEnumIds().end() );
}


bool mitk::EnumerationProperty::IsValidEnumerationValue( const std::string& val ) const
{
  return ( GetEnumStrings().find( val ) != GetEnumStrings().end() );
}
  

mitk::EnumerationProperty::EnumIdsContainerType& mitk::EnumerationProperty::GetEnumIds()
{
  std::string className = this->GetNameOfClass(); // virtual!
  return s_IdMapForClassName[ className ];
}


const mitk::EnumerationProperty::EnumIdsContainerType& mitk::EnumerationProperty::GetEnumIds() const
{
  std::string className = this->GetNameOfClass(); // virtual!
  return s_IdMapForClassName[ className ];
}


mitk::EnumerationProperty::EnumStringsContainerType& mitk::EnumerationProperty::GetEnumStrings()
{
  std::string className = this->GetNameOfClass(); // virtual!
  return s_StringMapForClassName[ className ];
}


const mitk::EnumerationProperty::EnumStringsContainerType& mitk::EnumerationProperty::GetEnumStrings() const
{
  std::string className = this->GetNameOfClass(); // virtual!
  return s_StringMapForClassName[ className ];
}

