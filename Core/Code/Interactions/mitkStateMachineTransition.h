/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#ifndef SMTRANSITION_H_HEADER_INCLUDED
#define SMTRANSITION_H_HEADER_INCLUDED

#include <MitkExports.h>
#include "mitkCommon.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <string>
#include "mitkStateMachineAction.h"

namespace mitk
{
  class StateMachineState;

  /**
   * @brief Connects two states, and holds references to actions that are executed on transition.
   *
   * @ingroup Interaction
   **/
  class MITK_CORE_EXPORT StateMachineTransition: public itk::Object
  {
  public:
    mitkClassMacro(StateMachineTransition, itk::Object);
    mitkNewMacro3Param(Self, std::string,std::string,std::string);

    typedef std::vector<mitk::StateMachineAction::Pointer> ActionVectorType;
    typedef itk::SmartPointer<StateMachineState> SpStateMachineState;

    void AddAction(StateMachineAction::Pointer action);
    SpStateMachineState GetNextState();
    std::string GetNextStateName() const;


    bool isEqual(StateMachineTransition::Pointer transition);
    /**
     * @brief Get an iterator on the first action in list.
     **/
    ActionVectorType GetActions() const;
    /*
     * @brief Returns true if the transition is triggered by the given event.
     **/
    bool MatchesEvent(std::string eventClass, std::string eventVariant) const;

    /**
     * @brief Set the next state of this object.
     **/
    void SetNextState(SpStateMachineState nextState);

  protected:
    StateMachineTransition(std::string nextStateName, std::string eventClass, std::string eventVariant);
    virtual ~StateMachineTransition();

    // Triggering Event
    std::string m_EventClass;
    std::string m_EventVariant;

  private:

    SpStateMachineState m_NextState;
    std::string m_NextStateName;


    /**
     * @brief The list of actions, that are executed if this transition is done.
     **/
    std::vector<StateMachineAction::Pointer> m_Actions;
  };

} // namespace mitk

#endif /* SMTRANSITION_H_HEADER_INCLUDED */
