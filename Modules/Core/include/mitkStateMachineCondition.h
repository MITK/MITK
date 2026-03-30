/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStateMachineCondition_h
#define mitkStateMachineCondition_h

#include <string>

namespace mitk
{
  /**
  * \brief Represents a condition, that has to be fulfilled in order to execute a state machine transition
  * after a certain event.
  *
  * This class represents a condition in the interaction framework. A condition corresponds
  * to a transition (StateMachineTransition) between two states (StateMachineState) and models
  * one requirement that has to be fulfilled in order to execute its transition.
  *
  * A condition returns either true or false. If the condition is to be inverted, the bool-flag
  * m_Inverted is set to true.
  *
  * example:
  * \code
  * <state name="dummy-state">
  *   <transition event="dummy-event" target="dummy-state">
  *     <condition name="dummy_condition_fulfilled" />  <!-- condition is fulfilled -->
  *     <action name"do_something" >
  *   </transition>
  *   <transition event="dummy-event" target="dummy-state">
  *     <condition name="dummy_condition_fulfilled" inverted="true"/>  <!-- condition is NOT fulfilled -->
  *     <action name"do_something_else" >
  *   </transition>
  * </state>
  * \endcode
  *
  * \ingroup Interaction
  */
  class StateMachineCondition
  {
  public:
    /**
     * \brief Construct a condition with the given name and inversion flag.
     *
     * \param[in] conditionName the string identifier for this condition.
     * \param[in] inverted if true, the condition result is logically inverted.
     */
    StateMachineCondition(const std::string &conditionName, const bool inverted);

    ~StateMachineCondition();

    /**
     * \brief Return the string identifier of this condition.
     * \return The condition name.
     */
    std::string GetConditionName() const;

    /**
     * \brief Check whether this condition is inverted.
     * \return True if the condition result should be logically negated.
     */
    bool IsInverted() const;

  private:
    /**
     * \brief The string identifier of this condition.
     */
    std::string m_ConditionName;

    /**
    * \brief Bool-flag that defines if this condition is to be inverted.
    */
    bool m_Inverted;
  };

} // namespace mitk

#endif
