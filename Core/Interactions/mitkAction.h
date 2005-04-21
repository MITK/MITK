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


#ifndef ACTION_H_HEADER_INCLUDED_C19AE06B
#define ACTION_H_HEADER_INCLUDED_C19AE06B

#include "mitkCommon.h"
#include "mitkPropertyList.h"


namespace mitk {
  //##Documentation
  //## @brief represents an action, that is executed after a certain event (in statemachine-mechanism)
  //##
  //## @ingroup Interaction
  class Action 
  {
    int m_ActionId;
    PropertyList::Pointer m_PropertiesList;

  public:
    Action( int actionId );
    ~Action();

    void AddProperty(const char* propertyKey, BaseProperty* property);

    int GetActionId() const;
    mitk::BaseProperty* GetProperty( const char *propertyKey ) const;
  };

} // namespace mitk

#endif /* ACTION_H_HEADER_INCLUDED_C19AE06B */
