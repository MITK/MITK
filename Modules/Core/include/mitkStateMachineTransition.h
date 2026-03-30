/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStateMachineTransition_h
#define mitkStateMachineTransition_h

#include <mitkCommon.h>
#include <mitkInteractionEvent.h>
#include <mitkStateMachineAction.h>
#include <mitkStateMachineCondition.h>
#include <itkLightObject.h>
#include <string>

#include <MitkCoreExports.h>

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
    mitkNewMacro3Param(Self, const std::string &, const std::string &, const std::string &);

    /**
     * \brief Return the next state that this transition leads to.
     * \return Smart pointer to the next state.
     */
    SpStateMachineState GetNextState() const;

    /**
     * \brief Return the name of the next state that this transition leads to.
     * \return The next state name.
     */
    std::string GetNextStateName() const;

    /**
     * \brief Check for equality with another transition.
     *
     * Equality is given if the event variant is the same and
     * classes are the same or the first argument is a superclass of the second.
     *
     * \warning The order of arguments matters.
     *
     * \param[in] transition the transition to compare with.
     * \return True if the transitions match.
     */
    bool operator==(const StateMachineTransition &transition) const;

    /**
     * \brief Return the list of actions associated with this transition.
     * \return A vector of actions to execute when this transition fires.
     */
    ActionVectorType GetActions() const;

    /**
     * \brief Return the list of conditions associated with this transition.
     * \return A const reference to the vector of conditions.
     */
    const ConditionVectorType &GetConditions() const;

    /**
     * \brief Set the next state of this transition.
     *
     * \param[in] nextState the state to transition to.
     */
    void SetNextState(const SpStateMachineState &nextState);

  protected:
    StateMachineTransition(const std::string &nextStateName,
                           const std::string &eventClass,
                           const std::string &eventVariant);
    ~StateMachineTransition() override;

    // Triggering Event
    std::string m_EventClass;
    std::string m_EventVariant;

  private:
    /**
     * \brief Add an action to be executed when this transition fires.
     *
     * \param[in] action the action to add.
     */
    void AddAction(const StateMachineAction::Pointer &action);

    /**
     * \brief Add a condition that must be fulfilled for this transition.
     *
     * \param[in] condition the condition to add.
     */
    void AddCondition(const StateMachineCondition &condition);

    SpStateMachineState m_NextState;

    std::string m_NextStateName;

    InteractionEvent::Pointer m_TransitionEvent;

    /**
     * \brief The list of actions that are executed when this transition is taken.
     */
    std::vector<StateMachineAction::Pointer> m_Actions;

    ConditionVectorType m_Conditions;
  };

} // namespace mitk

#endif
