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


#ifndef TRANSITION_H_HEADER_INCLUDED_C19AE06B
#define TRANSITION_H_HEADER_INCLUDED_C19AE06B

#include <MitkCoreExports.h>
#include <string>
#include <vector>
#include <itkWeakPointer.h>
#include "mitkAction.h"


namespace mitk {
  class State;

  /**
  * @brief Connection of two states
  *
  * A transition connects two states.
  * Several actions are stored, that have to be executed after the statechange.
  * @ingroup Interaction
  *
  * \deprecatedSince{2013_03} mitk::Transition is deprecated. Use mitk::StateMachineTransition instead.
  * Refer to \see DataInteractionPage for general information about the concept of the new implementation.
  */

  class MITKCORE_EXPORT Transition
  {
  public:

    typedef std::vector< mitk::Action::Pointer > ActionVectorType;
    typedef ActionVectorType::iterator ActionVectorIterator;
    typedef const ActionVectorIterator ActionVectorConstIterator;

    /**
    * @brief Add the action to this object.
    **/
    void AddAction( Action* action );

    /**
    * @brief Return the name of this object.
    **/
    std::string GetName() const;

    /**
    * @brief Get the next state of this object.
    **/
    State* GetNextState() const;

    /**
    * @brief Get the Id of the next state of this object.
    **/
    int GetNextStateId() const;

    /**
    * @brief Get the eventId of this object.
    **/
    int GetEventId() const;

    /**
    * @brief Get the number of actions.
    **/
    unsigned int GetActionCount() const;

    /**
    * @brief Get an interator on the first action in list.
    **/
    ActionVectorIterator GetActionBeginIterator() const;

    /**
    * @brief Get an interator behind the last action in list.
    **/
    ActionVectorConstIterator GetActionEndIterator() const;

    /**
    * @brief Returns true if the given eventId is equal to this eventId.
    **/
    bool IsEvent(int eventId) const;

    /**
    * @brief Set the next state of this object.
    **/
    void SetNextState(State* state);

    /**
    * @brief Default Constructor
    * Sets the necessary informations name (to enhance readability during debug),
    * nextStateId (the Id of the next state) and eventId (the Id of the event that causes the statechange).
    **/
    Transition(std::string name, int nextStateId, int eventId);

    /**
    * @brief Default Denstructor
    **/
    ~Transition();

  private:
    /**
    * @brief For better debugging and reading it stores the name of this.
    **/
    std::string m_Name;

    /**
    * @brief a Pointer to the next state of this object.
    **/
    itk::WeakPointer<mitk::State> m_NextState;

    /**
    * @brief The Id of the next state.
    **/
    int m_NextStateId;

    /**
    * @brief The eventId which is associated to this object.
    **/
    int m_EventId;

    /**
    * @brief The list of actions, that are executed if this transition is done.
    **/
    mutable std::vector<Action::Pointer> m_Actions;

  };

} // namespace mitk



#endif /* TRANSITION_H_HEADER_INCLUDED_C19AE06B */
