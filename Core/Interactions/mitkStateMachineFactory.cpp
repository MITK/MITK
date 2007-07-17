/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkStateMachineFactory.h"
#include "mitkGlobalInteraction.h"
#include <mitkStatusBar.h>
#include <vtkXMLDataElement.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkConfig.h>
#include <mitkStandardFileLocations.h>

#ifdef INTERACTION_DEBUG
#include <mitkInteractionDebug.h>
#endif

/**
* @brief This class builds up all the necessary structures for a statemachine.
* and stores one start-state for all built statemachines.
**/
mitk::StateMachineFactory::StartStateMap mitk::StateMachineFactory::m_StartStates;
mitk::StateMachineFactory::AllStateMachineMapType mitk::StateMachineFactory::m_AllStateMachineMap;
std::string mitk::StateMachineFactory::s_LastLoadedBehavior;

//XML StateMachine
const std::string mitk::StateMachineFactory::STYLE = "STYLE";
const std::string mitk::StateMachineFactory::NAME = "NAME";
const std::string mitk::StateMachineFactory::ID = "ID";	  
const std::string mitk::StateMachineFactory::START_STATE = "START_STATE";
const std::string mitk::StateMachineFactory::NEXT_STATE_ID = "NEXT_STATE_ID";
const std::string mitk::StateMachineFactory::EVENT_ID = "EVENT_ID";
const std::string mitk::StateMachineFactory::SIDE_EFFECT_ID = "SIDE_EFFECT_ID";
const std::string mitk::StateMachineFactory::ISTRUE = "TRUE";
const std::string mitk::StateMachineFactory::ISFALSE = "FALSE";
const std::string mitk::StateMachineFactory::STATE_MACHINE = "stateMachine";
const std::string mitk::StateMachineFactory::STATE = "state";
const std::string mitk::StateMachineFactory::TRANSITION = "transition";
const std::string mitk::StateMachineFactory::STATE_MACHINE_NAME = "stateMachine";
const std::string mitk::StateMachineFactory::ACTION = "action";
const std::string mitk::StateMachineFactory::BOOL_PARAMETER = "boolParameter";
const std::string mitk::StateMachineFactory::INT_PARAMETER = "intParameter";
const std::string mitk::StateMachineFactory::FLOAT_PARAMETER = "floatParameter";
const std::string mitk::StateMachineFactory::DOUBLE_PARAMETER = "doubleParameter";
const std::string mitk::StateMachineFactory::STRING_PARAMETER = "stringParameter";
const std::string mitk::StateMachineFactory::VALUE = "VALUE";

mitk::StateMachineFactory::StateMachineFactory()
: m_AktStateMachineName("")
{}

mitk::StateMachineFactory::~StateMachineFactory()
{}


/**
* @brief Returns NULL if no entry with string type is found.
**/
mitk::State* mitk::StateMachineFactory::GetStartState(const char * type)
{
  StartStateMapIter tempState = m_StartStates.find(type);
  if( tempState != m_StartStates.end() )
    return (tempState)->second.GetPointer();
  else
  {
    if(m_StartStates.size() == 0)
    {
      mitk::GlobalInteraction::StandardInteractionSetup();
      tempState = m_StartStates.find(type);
      if( tempState != m_StartStates.end() )
        return (tempState)->second.GetPointer();
    }
    itkGenericOutputMacro(<< "Start state not found for state-machine \"" << type << "\".");
  }
  return NULL;
}

/**
* @brief Loads the xml file filename and generates the necessary instances.
**/
bool mitk::StateMachineFactory::LoadBehavior(std::string fileName)
{
  if ( fileName.empty() )
    return false;

  s_LastLoadedBehavior = fileName;

  //call a new instance of this class and let it build up static containers
  mitk::StateMachineFactory* stateMachineFactory = new StateMachineFactory();
  stateMachineFactory->SetFileName( fileName.c_str() );

#ifdef INTERACTION_DEBUG
  InteractionDebug::SetXMLFileName( fileName.c_str() );
  InteractionDebug::GetInstance()->OpenConection();
#endif

  //parse the XML input. Method is implemented in vtkXMLParser
  if ( ! stateMachineFactory->Parse() )    
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText( "Could not parse behavior!" );
  }

  stateMachineFactory->Delete();
  return true;
}

bool mitk::StateMachineFactory::LoadStandardBehavior()
{
  std::string xmlFileName( mitk::StandardFileLocations::GetInstance()->FindFile("StateMachine.xml", "Core/Interactions") );

  if (!xmlFileName.empty()) 
    return LoadBehavior(xmlFileName);
  else
    return false;
}


/**
* @brief Recusive method, that parses this brand of 
* the stateMachine; if the history has the same 
* size at the end, then the StateMachine is correct
**/
bool mitk::StateMachineFactory::RParse(mitk::State::StateMap* states, mitk::State::StateMapIter thisState, HistorySet *history)
{
  history->insert((thisState->second)->GetId());//log our path  //or thisState->first. but this seems safer
  std::set<int> nextStatesSet = (thisState->second)->GetAllNextStates();

  //for debugging
  //	int thisStateId = (thisState->second)->GetId();
  //	int firstNextState = *nextStatesSet.begin();


  //remove loops in nextStatesSet; 
  //nether do we have to go there, nor will it clear a deadlock
  std::set<int>::iterator position = nextStatesSet.find((thisState->second)->GetId());//look for the same state in nextStateSet
  if (position != nextStatesSet.end())
  {//found the same state we are in!
    nextStatesSet.erase(position);//delete it, cause, we don't have to go there a second time!
  }

  //nextStatesSet is empty, so deadlock!
  if ( nextStatesSet.empty() )
  {
    StatusBar::GetInstance()->DisplayText("Warnung: Ein inkonsistenter Zustand (oder ein Endzustand) wird erzeugt!");    
    return true;//but it is allowed as an end-state
  }
  bool ok = false;
  //go through all Transitions of thisState and look if we have allready been in this state
  for (std::set<int>::iterator i = nextStatesSet.begin(); i != nextStatesSet.end();  i++)
  {
    if ( history->find(*i) == history->end() )//if we haven't been in this nextstate
    {
      mitk::State::StateMapIter nextState = states->find(*i);//search the iterator for our nextState
      if (nextState == states->end())
      {
        std::cout<<std::endl<<"Didn't find a state in StateMap! Check your statemachine behavior file!"<<std::endl;
        //you don't really see the warning in output! itkWarningMacro not possible due to this pointer!
        ok = false;
      }
      else
        ok = RParse( states, nextState, history);//recusive path into the next state
    }
  }
  return ok;
}

/**
* @brief sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
**/
bool mitk::StateMachineFactory::ConnectStates(mitk::State::StateMap *states)
{
  if (states->size() > 1)//only one state; don't have to be parsed for deadlocks!
  {
    //parse all the given states an check for deadlock or not connected states
    HistorySet *history = new HistorySet;
    mitk::State::StateMapIter firstState = states->begin(); 
    //parse through all the given states, log the parsed elements in history
    bool ok = RParse( states, firstState, history);

    if ( (states->size() == history->size()) && ok )
    {
      delete history;
    }
    else //ether !ok or sizeA!=sizeB
    {
      delete history;
      mitk::StatusBar::GetInstance()->DisplayText("Warning: An unreachable state was produced! Please check the StateMachinePattern-File.");    
      //return false;//better go on and build/ connect the states than quit
    }
  }
  //connect all the states
  for (mitk::State::StateMapIter tempState = states->begin(); tempState != states->end();  tempState++)
  {
    //searched through the States and Connects all Transitions
    bool tempbool = ( ( tempState->second )->ConnectTransitions( states ) );
    if ( tempbool = false )
    {
      mitk::StatusBar::GetInstance()->DisplayText("Warning: The connection of the states was not successful!");    
      return false;//abort!
    }
  }
  return true;
}

void  mitk::StateMachineFactory::StartElement (const char* elementName, const char **atts) 
{
  std::string name(elementName);

  if ( name == STATE_MACHINE )
  { 
    m_AktStateMachineName = ReadXMLStringAttribut( NAME, atts ); 
    m_AllStateMachineMap[ m_AktStateMachineName ] = new StateMachineMapType;
  } 

  else if ( name == STATE )   
  {
    std::string stateMachinName = ReadXMLStringAttribut( NAME, atts ) ;
    int id = ReadXMLIntegerAttribut( ID, atts );

    //create a new instance
    m_AktState = mitk::State::New(stateMachinName , id);
    
    // store all states to be able to access a specific state (for persistence)
    StateMachineMapType* stateMachine = m_AllStateMachineMap[ m_AktStateMachineName ];
    (*stateMachine)[id] = m_AktState;

    std::pair<mitk::State::StateMapIter,bool> ok = m_AllStatesOfOneStateMachine.insert(mitk::State::StateMap::value_type(id , m_AktState));

    if ( ok.second == false ) 
    { 
      std::cout<<std::endl;
      std::cout<<"Warning from StateMachineFactory: STATE_ID was not unique or something else didn't work in insert!"<<std::endl;
      return; //STATE_ID was not unique or something else didn't work in insert! EXITS the process
    }
    if ( ReadXMLBooleanAttribut( START_STATE, atts ) )
      m_StartStates.insert(StartStateMap::value_type(m_AktStateMachineName, m_AktState));  
  } 

  else if ( name == TRANSITION )   
  {
    std::string transitionName = ReadXMLStringAttribut( NAME, atts ) ;
    int nextStateId = ReadXMLIntegerAttribut( NEXT_STATE_ID, atts );
    int eventId = ReadXMLIntegerAttribut( EVENT_ID, atts );
    m_AktTransition = Transition::New(transitionName, nextStateId, eventId);
    if ( m_AktState )
      m_AktState->AddTransition( m_AktTransition );  
  }

  else if ( name == ACTION )
  {
    int actionId = ReadXMLIntegerAttribut( ID, atts );
    m_AktAction = Action::New( actionId );
    m_AktTransition->AddAction( m_AktAction );  
  } 

  else if ( name == BOOL_PARAMETER )
  {
    if ( !m_AktAction )
      return;

    bool value = ReadXMLBooleanAttribut( VALUE, atts );    
    std::string name = ReadXMLStringAttribut( NAME, atts );    
    m_AktAction->AddProperty( name.c_str(), new BoolProperty( value ) );  
  }  

  else if ( name == INT_PARAMETER )
  {
    if ( !m_AktAction )
      return;

    int value = ReadXMLIntegerAttribut( VALUE, atts );    
    std::string name = ReadXMLStringAttribut( NAME, atts );    
    m_AktAction->AddProperty( name.c_str(), new IntProperty( value ) );  
  }

  else if ( name == FLOAT_PARAMETER )
  {
    if ( !m_AktAction )
      return;

    float value = ReadXMLIntegerAttribut( VALUE, atts ); 
    std::string name = ReadXMLStringAttribut( NAME, atts );    
    m_AktAction->AddProperty( name.c_str(), new FloatProperty( value ) );    
  }

  else if ( name == DOUBLE_PARAMETER )
  {
    if ( !m_AktAction )
      return;

    double value = ReadXMLDoubleAttribut( VALUE, atts );
    std::string name = ReadXMLStringAttribut( NAME, atts );    
    m_AktAction->AddProperty( name.c_str(), new DoubleProperty( value ) );      
  }

  else if ( name == STRING_PARAMETER )
  {
    if ( !m_AktAction )
      return;

    std::string value = ReadXMLStringAttribut( VALUE, atts );
    std::string name = ReadXMLStringAttribut( NAME, atts );    
    m_AktAction->AddProperty( name.c_str(), new StringProperty( value ) );        
  }
}

void mitk::StateMachineFactory::EndElement (const char* elementName) 
{
  bool ok = true;
  std::string name(elementName);

  if ( name == STATE_MACHINE_NAME )
  {
    ok = ConnectStates(&m_AllStatesOfOneStateMachine);
    m_AllStatesOfOneStateMachine.clear();
  } 
  else if ( name == STATE_MACHINE ) 
  {
    //doesn't have to be done
    // m_AktState = NULL;
    // m_StartStates = NULL;
  } 
  else if ( name == TRANSITION ) 
  {
    m_AktTransition = NULL;
  } 
  else if ( name == ACTION ) 
  {
    m_AktAction = NULL;
  }
}

std::string mitk::StateMachineFactory::ReadXMLStringAttribut( std::string name, const char** atts )
{
  if(atts)
  {
    const char** attsIter = atts;  

    while(*attsIter) 
    {     
      if ( name == *attsIter )
      {
        attsIter++;      
        return *attsIter;
      }
      attsIter++;
      attsIter++;
    }
  }

  return std::string();
}

int mitk::StateMachineFactory::ReadXMLIntegerAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );
  return atoi( s.c_str() );
}

float mitk::StateMachineFactory::ReadXMLFloatAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );
  return (float) atof( s.c_str() );
}

double mitk::StateMachineFactory::ReadXMLDoubleAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );
  return atof( s.c_str() );
}

bool mitk::StateMachineFactory::ReadXMLBooleanAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );

  if ( s == ISTRUE )
    return true;
  else
    return false;
}

mitk::State* mitk::StateMachineFactory::GetState( const char * type, int StateId )
{
  //check if the state exists
  AllStateMachineMapType::iterator i = m_AllStateMachineMap.find( type );
  if ( i == m_AllStateMachineMap.end() )
    return false;

  //get the statemachine of the state
  StateMachineMapType* sm = m_AllStateMachineMap[type];

  //get the state from its statemachine
  if ( sm != NULL )
    return (*sm)[StateId].GetPointer();  
  else
    return NULL;
}

