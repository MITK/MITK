/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
 *
 */
Action::Action( int actionId )
:m_ActionId( actionId )
{ 
  m_PropertiesList = PropertyList::New();
}

/**
 *
 */
Action::~Action() 
{
}

/**
 *
 */
void Action::AddProperty(const char* propertyKey, BaseProperty* property ) 
{
  m_PropertiesList->SetProperty( propertyKey, property );
}


/**
 *
 */
int Action::GetActionId() const
{
  return m_ActionId;
}


/**
 *
 */
mitk::BaseProperty* Action::GetProperty( const char *propertyKey ) const 
{
  return m_PropertiesList->GetProperty( propertyKey );
}

} // namespace mitk
