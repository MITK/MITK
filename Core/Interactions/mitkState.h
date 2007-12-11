/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef STATE_H_HEADER_INCLUDED_C19A8A5D
#define STATE_H_HEADER_INCLUDED_C19A8A5D

#include "mitkCommon.h"
#include <string>
#include <map>
#include <set>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include "mitkTransition.h"

namespace mitk {

  /**
  * @brief represents one state with all its necessary information
  *
  * Name and ID are stored. Also methods for building up, connecting and
  * parsing for well formed statemachines are present.
  * This class holds a map of transitions to next States. 
  * @ingroup Interaction
  **/
  class State : public itk::Object
  {
  public:
    mitkClassMacro(State, itk::Object);

    /**
    * @brief static New method to use SmartPointer
    **/
    mitkNewMacro2Param(Self, std::string, int);

    typedef std::map<int, mitk::State::Pointer> StateMap;
    typedef std::map<int, mitk::Transition::Pointer> TransitionMap;

    typedef StateMap::iterator StateMapIter;
    typedef TransitionMap::iterator TransMapIter;

    typedef TransitionMap::const_iterator TransMapConstIter;

    /**
    * @brief Add a transition to the map of transitions.
    **/
    bool AddTransition( Transition* transition );

    /**
    * @brief hashmap-lookup and returning the Transition. Returns NULL Pointer if not located
    **/
    const Transition* GetTransition(int eventId) const;

    /**
    * @brief Returns the name.
    **/
    std::string GetName() const;

    /**
    * @brief Returns the Id.
    **/
    int GetId() const;

    /**
    * @brief Returns a set of all next States. E.g. to parse through all States.
    **/
    std::set<int> GetAllNextStates() const;

    /**
    * @brief Check, if this event (eventId) leads to a state. 
    **/
    bool IsValidEvent(int eventId) const;

    /**
    * @brief Searches dedicated States of all Transitions and sets *nextState of these Transitions. 
    * Required for this is a List of all build States of that StateMachine (allStates). This way the StateMachine can be build up.
    **/
    bool ConnectTransitions(StateMap* allStates);

  protected:
    /**
    * @brief Default Constructor. Use ::New instead!
    * Set the name and the Id of the state. 
    * Name is to maintain readability during debug and Id is to identify this state inside the StateMachinePattern
    **/
    State(std::string name, int id);
    
    /**
    * @brief Default Destructor
    **/
    ~State(){}

  private:
    /**
    * @brief Name of this State to support readability during debug
    **/
    std::string m_Name;

    /**
    * @brief Id of this State important for interaction mechanism in StateMachinePattern
    *
    * All states inside a StateMachinePattern (construct of several states connected with transitions and actions) have to have an own id with which it is identifyable.
    **/
    int m_Id;

    /**
    * @brief map of transitions that lead from this state to the next state
    **/
    TransitionMap m_Transitions;

  };

} // namespace mitk



#endif /* STATE_H_HEADER_INCLUDED_C19A8A5D */
