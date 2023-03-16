/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStateMachineAction_h
#define mitkStateMachineAction_h

#include "MitkCoreExports.h"
#include "mitkCommon.h"
#include <itkLightObject.h>
#include <itkObjectFactory.h>
#include <string>

namespace mitk
{
  //##Documentation
  //## @brief Represents an action, that is executed after a certain event (in statemachine-mechanism)
  //## TODO: implement: Is used to connect the Action-Id in an XML Statemachine description with a functor in the
  // StateMachine
  //## implementation
  //## @ingroup Interaction
  class MITKCORE_EXPORT StateMachineAction : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(StateMachineAction, itk::Object);
    mitkNewMacro1Param(Self, const std::string&);

    /**
      * @brief Returns the String-Id of this action.
      **/
    std::string GetActionName() const;

  protected:
    StateMachineAction(const std::string &);
    ~StateMachineAction() override;

  private:
    /**
     * @brief The Id-Name of this action.
     **/
    std::string m_ActionName;
  };

} // namespace mitk

#endif
