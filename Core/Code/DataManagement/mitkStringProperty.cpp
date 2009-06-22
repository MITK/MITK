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


#include "mitkStringProperty.h"

const char* mitk::StringProperty::PATH = "path";
mitk::StringProperty::StringProperty( const char* string ) 
: m_Value()
{
  if ( string )
    m_Value = string;
}

mitk::StringProperty::StringProperty( const std::string& s )
: m_Value( s )
{

}

bool mitk::StringProperty::operator==(const BaseProperty& property ) const 
{
  const Self *other = dynamic_cast<const Self*>(&property);

  if(other==NULL) return false;

  return other->m_Value==m_Value;
}

std::string mitk::StringProperty::GetValueAsString() const 
{
  return m_Value;
}

bool mitk::StringProperty::Assignable(const BaseProperty& other) const
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

mitk::BaseProperty& mitk::StringProperty::operator=(const BaseProperty& other)
{
  try
  {
    const Self& otherProp( dynamic_cast<const Self&>(other) );

    if (this->m_Value != otherProp.m_Value)
    {
      this->m_Value = otherProp.m_Value;
      this->Modified();
    }
  }
  catch (std::bad_cast)
  {
    // nothing to do then
  }

  return *this;
}


