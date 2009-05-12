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

#include "mitkAction.h"

namespace mitk {

/**
 * Default Constructor
 */
Action::Action( int actionId )
:m_ActionId( actionId ), m_PropertiesList(NULL)
{}

Action::~Action()
{}

/**
 * add a property
 */
void Action::AddProperty(const char* propertyKey, BaseProperty* property ) 
{
  if (m_PropertiesList.IsNull())
    m_PropertiesList = PropertyList::New();
  m_PropertiesList->SetProperty( propertyKey, property );
}


/**
 * return the actionId of this object
 */
int Action::GetActionId() const
{
  return m_ActionId;
}


/**
 * return the property with the given property key
 */
mitk::BaseProperty* Action::GetProperty( const char *propertyKey ) const 
{
  if (m_PropertiesList.IsNotNull())
    return m_PropertiesList->GetProperty( propertyKey );
  else 
    return NULL;
}

} // namespace mitk
