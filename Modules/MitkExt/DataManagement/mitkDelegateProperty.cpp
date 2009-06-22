/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14605 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkDelegateProperty.h"

mitk::DelegateProperty::DelegateProperty( const char* string ) 
: mitk::StringProperty(string)
{
  if ( string )
    m_Value = string;
}

mitk::DelegateProperty::DelegateProperty( const std::string& s )
: mitk::StringProperty(s)
{

}

bool mitk::DelegateProperty::operator==(const DelegateProperty& property ) const 
{
  const Self *other = dynamic_cast<const Self*>(&property);

  if(other==NULL) return false;

  return other->m_Value==m_Value;
}

mitk::DelegateProperty& mitk::DelegateProperty::operator=(const DelegateProperty& other)
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


