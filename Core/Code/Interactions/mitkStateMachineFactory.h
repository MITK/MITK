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


#ifndef STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD
#define STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD

#include "mitkCommon.h"
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
  * Instance of a new StateMachine grabs a StartState of one certain
  * statemachine. Two instances of one kind of statemachine share that
  * statemachine. 
  * During buildprocess at runtime each statemachine is parsed for well formed style.
  * @ingroup Interaction
  **/
  class MITK_CORE_EXPORT StateMachineFactory : public vtkXMLParser
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
    static State* GetStartState(const char* type);

    /**
    * @brief loads the xml file filename and generates the necessary instances
    **/
    static bool LoadBehavior(std::string fileName);

    /**
    * @brief Try to load standard behavior file "StateMachine.xml"
    *
    * Search strategy:
    * \li try environment variable "MITKCONF" (path to "StateMachine.xml")
    * \li try "./StateMachine.xml"
    * \li try via source directory (using MITKROOT from cmake-created 
    * mitkConfig.h) "MITKROOT/Interactions/mitkBaseInteraction/StateMachine.xml"
    **/
    static bool LoadStandardBehavior();

    static const std::string& GetLastLoadedBehavior()
    {
      return s_LastLoadedBehavior;
    }

    /**
    * @brief Derived from XMLReader
    **/
    void  StartElement (const char* elementName, const char **atts);

    /**
    * @brief Derived from XMLReader
    **/
    void  EndElement (const char* elementName);

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
    static mitk::State* GetState( const char* type, int StateId );

    /**
    * @brief Sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
    **/
    static bool ConnectStates(mitk::State::StateMap* states);

    /**
    * @brief Recusive method, that parses this pattern of the stateMachine and returns true if correct
    **/
    static bool RParse(mitk::State::StateMap* states, mitk::State::StateMapIter thisState, HistorySet *history);

     /**
    * @brief Holds all created States that are defined as StartState
    **/
    static StartStateMap m_StartStates;

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
    itk::WeakPointer<Transition> m_AktTransition;

    /**
    * @brief A pointer to an Action to help building up the pattern
    **/
    Action::Pointer m_AktAction;

    /**
    * @brief map to hold all statemachines to call GetState for friends
    **/
    static AllStateMachineMapType m_AllStateMachineMap;

    static std::string s_LastLoadedBehavior;

    std::string m_AktStateMachineName;

    static const std::string STYLE;
    static const std::string NAME;
    static const std::string ID;    
    static const std::string START_STATE;
    static const std::string NEXT_STATE_ID;
    static const std::string EVENT_ID;
    static const std::string SIDE_EFFECT_ID;
    static const std::string ISTRUE;
    static const std::string ISFALSE;
    static const std::string STATE_MACHINE;
    static const std::string TRANSITION;    
    static const std::string STATE;
    static const std::string STATE_MACHINE_NAME;
    static const std::string ACTION;
    static const std::string BOOL_PARAMETER;
    static const std::string INT_PARAMETER;
    static const std::string FLOAT_PARAMETER;
    static const std::string DOUBLE_PARAMETER;
    static const std::string STRING_PARAMETER;
    static const std::string VALUE;

  };

} // namespace mitk


#endif /* STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD */
