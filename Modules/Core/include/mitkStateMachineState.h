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

#ifndef SMSTATE_H_HEADER_INCLUDED_C19A8A5D
#define SMSTATE_H_HEADER_INCLUDED_C19A8A5D

#include <string>
#include <itkLightObject.h>
#include "mitkStateMachineTransition.h"
#include "MitkCoreExports.h"

namespace mitk {
  /**
   * \class StateMachineState
   * Represents a state of a state machine pattern.
   * It holds transitions to other states (mitk::StateMachineTransition) and the mode of the current state, see
   * m_StateMode .
   */

  class MITKCORE_EXPORT StateMachineState : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(StateMachineState, itk::LightObject);
    mitkNewMacro2Param(Self, const std::string&, const std::string&);



    typedef std::vector<mitk::StateMachineState::Pointer> StateMap;
    typedef std::vector<StateMachineTransition::Pointer> TransitionVector;

    bool AddTransition( StateMachineTransition::Pointer transition );

    /**
    * @brief Return Transition which matches given event description.
    *
    * \deprecatedSince{2013_09} Use method GetTransitionList() instead.
    */
    DEPRECATED(StateMachineTransition::Pointer GetTransition(const std::string& eventClass,const std::string& eventVariant));

    /**
    * @brief Return Transitions that match given event description.
    **/
    TransitionVector GetTransitionList(const std::string& eventClass,const std::string& eventVariant);

    /**
    * @brief Returns the name.
    **/
    std::string GetName() const;

    std::string GetMode() const;

    /**
    * @brief Searches dedicated States of all Transitions and sets *nextState of these Transitions.
    * Required for this is a List of all build States of that StateMachine (allStates). This way the StateMachine can be build up.
    **/
    bool ConnectTransitions(StateMap* allStates);

  protected:

    StateMachineState(const std::string& name, const std::string& stateMode);
    ~StateMachineState();

  private:
    /**
    * @brief Name of this State.
    **/
    std::string m_Name;
    /**
     * State Modus, which determines the behavior of the dispatcher. A State can be in three different modes:
     * REGULAR - standard dispatcher behavior
     * GRAB_INPUT - all events are given to the statemachine in this modus, if they are not processed by this statemachine the events are dropped.
     * PREFER_INPUT - events are first given to this statemachine, and if not processed, offered to the other statemachines.
     */
    std::string m_StateMode;

    /**
    * @brief map of transitions that lead from this state to the next state
    **/
    TransitionVector m_Transitions;
  };
} // namespace mitk
#endif /* SMSTATE_H_HEADER_INCLUDED_C19A8A5D */
