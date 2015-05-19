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

#include "mitkCommon.h"
#include <itkLightObject.h>
#include <string>
#include "mitkStateMachineAction.h"
#include "mitkStateMachineCondition.h"
#include "mitkInteractionEvent.h"

#include "MitkCoreExports.h"

namespace mitk
{
  class StateMachineState;

  typedef std::vector<mitk::StateMachineAction::Pointer> ActionVectorType;
  typedef std::vector<StateMachineCondition> ConditionVectorType;
  typedef itk::SmartPointer<StateMachineState> SpStateMachineState;

  /**
  * \class StateMachineTransition
  * \brief Connects two states, and holds references to corresponding actions and conditions.
  *
  * This class represents a transition between two states of a statemachine. It holds a
  * list of conditions that have to be fulfilled in order to be executed correctly.
  * It also holds a list of actions that will be executed if all conditions are fulfilled.
  *
  * \ingroup Interaction
  **/

  class MITKCORE_EXPORT StateMachineTransition : public itk::LightObject
  {
    friend class StateMachineFactory;
    friend class StateMachineContainer;

  public:
    mitkClassMacroItkParent(StateMachineTransition, itk::LightObject);
    mitkNewMacro3Param(Self, const std::string&, const std::string&, const std::string&);

    SpStateMachineState GetNextState() const;
    std::string GetNextStateName() const;
    /**
     * Check for equality. Equality is given if event variant is the same and
     * classes are the same or the first argument is a superclass of the second.
     * \warn Here the order of arguments matters. !
     */
    bool operator==(const StateMachineTransition& transition) const;


    /**
     * @brief Get an iterator on the first action in list.
     **/
    ActionVectorType GetActions() const;

    const ConditionVectorType& GetConditions() const;

    /**
     * @brief Set the next state of this object.
     **/
    void SetNextState(const SpStateMachineState& nextState);

  protected:
    StateMachineTransition(const std::string& nextStateName, const std::string& eventClass, const std::string& eventVariant);
    virtual ~StateMachineTransition();

    // Triggering Event
    std::string m_EventClass;
    std::string m_EventVariant;

  private:

    void AddAction(const StateMachineAction::Pointer& action);

    void AddCondition(const StateMachineCondition& condition);


    SpStateMachineState m_NextState;

    std::string m_NextStateName;

    InteractionEvent::Pointer m_TransitionEvent;

    /**
     * @brief The list of actions, that are executed if this transition is done.
     **/
    std::vector<StateMachineAction::Pointer> m_Actions;

    ConditionVectorType m_Conditions;
  };

} // namespace mitk

#endif /* SMTRANSITION_H_HEADER_INCLUDED */
