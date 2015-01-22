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

#ifndef StateMachineCondition_H_HEADER_INCLUDED_C19AE06B
#define StateMachineCondition_H_HEADER_INCLUDED_C19AE06B

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

    StateMachineCondition( const std::string&, const bool inverted );
    ~StateMachineCondition();

    /**
     * @brief Returns the String-Id of this action.
     **/
    std::string GetConditionName() const;

    bool IsInverted() const;

  private:
    /**
     * @brief The Id-Name of this action.
     **/
    std::string m_ConditionName;

    /**
    * \brief Bool-flag that defines if this condition is to be inverted.
    */
    bool m_Inverted;

  };

} // namespace mitk

#endif /* StateMachineCondition_H_HEADER_INCLUDED_C19AE06B */
