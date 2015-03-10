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


#ifndef STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD
#define STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD

#include <MitkCoreExports.h>
#include "mitkState.h"
#include "mitkTransition.h"
#include "mitkAction.h"
#include <vtkXMLParser.h>
#include <iostream>
#include <set>


namespace mitk {

  /**
  *@brief builds up all specifiyed statemachines and hold them for later access
  *
  * According to the XML-File every different statemachine is build up. A new
  * instance of a new StateMachine grabs a StartState of one certain
  * state machine. Two instances of one kind of state machine share that
  * state machine.
  * During buildprocess at runtime each state machine is parsed for well formed style.
  * Currently different interaction styles are not yet supported.
  * To add individual state machine patterns, call LoadBehavior(...)
  * and it will be parsed added to the internal list of patterns
  *
  * @ingroup Interaction
  **/
  /**
   * \deprecatedSince{2013_03} StateMachineFactory is deprecated. Please use mitk::StateMachineContainer instead.
   * Refer to \see DataInteractionPage for general information about the concept of the new implementation.
   */

  class MITKCORE_EXPORT StateMachineFactory : public vtkXMLParser
  {
  public:
    static StateMachineFactory *New();
    vtkTypeMacro(StateMachineFactory,vtkXMLParser);

    /**
    * @brief Typedef for all states that are defined as start-states
    **/
    typedef std::map<std::string, mitk::State::Pointer> StartStateMap;
    typedef StartStateMap::iterator                     StartStateMapIter;

    /**
    * @brief Typedef to be used for parsing all states of one statemachine
    **/
    typedef std::set<int>                               HistorySet;
    typedef HistorySet::iterator                        HistorySetIter;

    /**
    * @brief This type holds all states of one statemachine.
    **/
    typedef std::map<int,State::Pointer>                StateMachineMapType;

    /**
    * @brief this type holds all states of all statemachines so that a specific state can be accessed for persistence
    **/
    typedef std::map<std::string, StateMachineMapType* > AllStateMachineMapType;

    /**
    * @brief Returns the StartState of the StateMachine with the name type;
    *
    * Returns NULL if no entry with name type is found.
    * Here a Smartpointer is returned to ensure, that StateMachines are also considered during reference counting.
    **/
    State* GetStartState(const char* type);

    /**
    * @brief loads the xml file filename and generates the necessary instances
    **/
    bool LoadBehavior(std::string fileName);

    /**
    * @brief loads the xml string and generates the necessary instances
    **/
    bool LoadBehaviorString(std::string xmlString);

    /**
    * @brief Try to load standard behavior file "StateMachine.xml"
    *
    * Search strategy:
    * \li try environment variable "MITKCONF" (path to "StateMachine.xml")
    * \li try "./StateMachine.xml"
    * \li try via source directory (using MITKROOT from cmake-created
    * mitkConfig.h) "MITKROOT/Interactions/mitkBaseInteraction/StateMachine.xml"
    **/
    bool LoadStandardBehavior();

    const std::string& GetLastLoadedBehavior()
    {
      return m_LastLoadedBehavior;
    }

    /**
    * @brief Adds the given pattern to the internal list of patterns
    *
    * Method to support addition of externaly loaded patterns.
    * Instances of states, transitions and actions are maintained within this class and freed on destruction.
    * The states already have to be connected by transitions and actions and checked for errors.
    * \param type name of the pattern to add. Will be used during initialization of a new interactor.
    * \param startState the start state of this pattern.
    * \param allStatesOfStateMachine a map of state ids and its states to hold their reference and delete them in destructor
    **/
    bool AddStateMachinePattern(const char * type, mitk::State* startState, StateMachineMapType* allStatesOfStateMachine);


    /**
    * brief To enable StateMachine to access states
    **/
    friend class StateMachine;

  protected:
    /**
    * @brief Default Constructor
    **/
    StateMachineFactory();

    /**
    * @brief Default Destructor
    **/
    ~StateMachineFactory();

    /**
    * @brief Derived from XMLReader
    **/
    void  StartElement (const char* elementName, const char **atts);

    /**
    * @brief Derived from XMLReader
    **/
    void  EndElement (const char* elementName);

  private:
    /**
    * @brief Derived from XMLReader
    **/
    std::string ReadXMLStringAttribut( std::string name, const char** atts);
    /**
    * @brief Derived from XMLReader
    **/
    float ReadXMLFloatAttribut( std::string name, const char** atts );
    /**
    * @brief Derived from XMLReader
    **/
    double ReadXMLDoubleAttribut( std::string name, const char** atts );
    /**
    * @brief Derived from XMLReader
    **/
    int ReadXMLIntegerAttribut( std::string name, const char** atts );
    /**
    * @brief Derived from XMLReader
    **/
    bool ReadXMLBooleanAttribut( std::string name, const char** atts );

    /**
    * @brief Returns a Pointer to the desired state if found.
    **/
    mitk::State* GetState( const char* type, int StateId );

    /**
    * @brief Sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
    **/
    bool ConnectStates(mitk::State::StateMap* states);

    /**
    * @brief Recusive method, that parses this pattern of the stateMachine and returns true if correct
    **/
    bool RParse(mitk::State::StateMap* states, mitk::State::StateMapIter thisState, HistorySet *history);

     /**
    * @brief Holds all created States that are defined as StartState
    **/
    StartStateMap m_StartStates;

    /**
    * @brief Holds all States of one StateMachine to build up the pattern.
    **/
    mitk::State::StateMap m_AllStatesOfOneStateMachine;

    /**
    * @brief A pointer to a State to help building up the pattern
    **/
    State::Pointer m_AktState;

    /**
    * @brief A pointer to a Transition to help building up the pattern
    **/
    Transition* m_AktTransition;

    /**
    * @brief A pointer to an Action to help building up the pattern
    **/
    Action::Pointer m_AktAction;

    /**
    * @brief map to hold all statemachines to call GetState for friends
    **/
    AllStateMachineMapType m_AllStateMachineMap;

    std::string m_LastLoadedBehavior;

    std::string m_AktStateMachineName;

    /**
    * @brief Variable to skip a state machine pattern if the state machine name is not unique
    **/
    bool m_SkipStateMachine;
  };

} // namespace mitk


#endif /* STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD */
