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


#ifndef ACTION_H_HEADER_INCLUDED_C19AE06B
#define ACTION_H_HEADER_INCLUDED_C19AE06B

#include "mitkCommon.h"
#include "mitkPropertyList.h"
#include <itkObject.h>
#include <itkObjectFactory.h>


namespace mitk {
  //##Documentation
  //## @brief represents an action, that is executed after a certain event (in statemachine-mechanism)
  //##
  //## @ingroup Interaction
  class MITK_CORE_EXPORT Action : public itk::Object
  {
  public:
    mitkClassMacro(Action, itk::Object);

    /**
    * @brief static New method to use SmartPointer
    **/
    mitkNewMacro1Param(Self, int);
    
    /**
    * @brief Adds a property to the list of properties.
    **/
    void AddProperty(const char* propertyKey, BaseProperty* property);

    /**
    * @brief Returns the Id of this action.
    **/
    int GetActionId() const;

    /**
    * @brief returns the specified property
    **/
    mitk::BaseProperty* GetProperty( const char *propertyKey ) const;

  protected: 
    /**
    * @brief Default Constructor.
    * Set the actionId.
    **/
    Action( int actionId );

    /**
    * @brief Default Destructor
    **/
    ~Action();
  private:
    /**
    * @brief The Id of this action.
    **/
    int m_ActionId;

    /**
    * @brief An action can also have several properties that are needed to execute a special action.
    **/
    PropertyList::Pointer m_PropertiesList;
  };

} // namespace mitk

#endif /* ACTION_H_HEADER_INCLUDED_C19AE06B */
