/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAction_h
#define mitkAction_h

#include "mitkPropertyList.h"
#include <MitkCoreExports.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk
{
  //##Documentation
  //## @brief represents an action, that is executed after a certain event (in statemachine-mechanism)
  //##
  //## @ingroup Interaction
  class MITKCORE_EXPORT Action : public itk::Object
  {
  public:
    mitkClassMacroItkParent(Action, itk::Object);

    /**
    * @brief static New method to use SmartPointer
    **/
    mitkNewMacro1Param(Self, int);

    /**
    * @brief Adds a property to the list of properties.
    **/
    void AddProperty(const char *propertyKey, BaseProperty *property);

    /**
    * @brief Returns the Id of this action.
    **/
    int GetActionId() const;

    /**
    * @brief returns the specified property
    **/
    mitk::BaseProperty *GetProperty(const char *propertyKey) const;

  protected:
    /**
    * @brief Default Constructor.
    * Set the actionId.
    **/
    Action(int actionId);

    /**
    * @brief Default Destructor
    **/
    ~Action() override;

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

#endif
