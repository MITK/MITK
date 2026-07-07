/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStateMachineState_h
#define mitkStateMachineState_h

#include <MitkCoreExports.h>
#include <mitkStateMachineTransition.h>
#include <itkLightObject.h>
#include <string>

namespace mitk
{
  /**
   * \brief Represents a state of a state machine pattern.
   *
   * It holds transitions to other states (mitk::StateMachineTransition) and the mode
   * of the current state, which determines how the dispatcher handles events.
   *
   * \ingroup Interaction
   */

  class MITKCORE_EXPORT StateMachineState : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(StateMachineState, itk::LightObject);
    mitkNewMacro2Param(Self, const std::string &, const std::string &);

    typedef std::vector<mitk::StateMachineState::Pointer> StateMap;
    typedef std::vector<StateMachineTransition::Pointer> TransitionVector;

    /**
     * \brief Add a transition to this state.
     *
     * \param[in] transition the transition to add.
     * \return True if the transition was added, false if it already exists.
     */
    bool AddTransition(StateMachineTransition::Pointer transition);

    /**
     * \brief Return all transitions matching the given event description.
     *
     * \param[in] eventClass the event class to match.
     * \param[in] eventVariant the event variant to match.
     * \return A vector of matching transitions.
     */
    TransitionVector GetTransitionList(const std::string &eventClass, const std::string &eventVariant);

    /**
     * \brief Return the name of this state.
     * \return The state name.
     */
    std::string GetName() const;

    /**
     * \brief Return the mode of this state.
     *
     * The mode can be "REGULAR", "GRAB_INPUT", or "PREFER_INPUT".
     *
     * \return The state mode string.
     */
    std::string GetMode() const;

    /**
     * \brief Connect all transitions to their target states by resolving state name references.
     *
     * Post-processing step when building a state machine from XML. Iterates all transitions
     * and finds the State that matches each transition's target state name.
     *
     * \param[in] allStates a list of all states in the state machine.
     * \return True if all transitions were connected successfully, false if any target state was not found.
     */
    bool ConnectTransitions(StateMap *allStates);

  protected:
    StateMachineState(const std::string &name, const std::string &stateMode);
    ~StateMachineState() override;

  private:
    /** \brief Name of this state. */
    std::string m_Name;

    /**
     * \brief State mode, which determines the behavior of the dispatcher.
     *
     * A state can be in three different modes:
     * - REGULAR: standard dispatcher behavior.
     * - GRAB_INPUT: all events are given to the state machine in this mode; if not
     *   processed, the events are dropped.
     * - PREFER_INPUT: events are first given to this state machine, and if not
     *   processed, offered to other state machines.
     */
    std::string m_StateMode;

    /** \brief Transitions that lead from this state to other states. */
    TransitionVector m_Transitions;
  };
} // namespace mitk
#endif
