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
#include <itkLightObject.h>
#include <itkObjectFactory.h>
#include <string>
#include "mitkStateMachineAction.h"
#include "mitkInteractionEvent.h"

namespace mitk
{
  class StateMachineState;

    typedef std::vector<mitk::StateMachineAction::Pointer> ActionVectorType;
    typedef itk::SmartPointer<StateMachineState> SpStateMachineState;

    /**
     * \class StateMachineTransition
   * @brief Connects two states, and holds references to actions that are executed on transition.
   *
   * @ingroup Interaction
   **/

    class MITK_CORE_EXPORT StateMachineTransition: public itk::LightObject
  {
  public:
    mitkClassMacro(StateMachineTransition, itk::LightObject);
    mitkNewMacro3Param(Self, const std::string&, const std::string&, const std::string&);


    void AddAction(const StateMachineAction::Pointer& action);
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

    SpStateMachineState m_NextState;
    std::string m_NextStateName;


    InteractionEvent::Pointer m_TransitionEvent;

    /**
     * @brief The list of actions, that are executed if this transition is done.
     **/
    std::vector<StateMachineAction::Pointer> m_Actions;
  };

} // namespace mitk

#endif /* SMTRANSITION_H_HEADER_INCLUDED */
