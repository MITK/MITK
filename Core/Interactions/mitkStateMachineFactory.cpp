#include "mitkStateMachineFactory.h"
#include "mitkStatusBar.h"
#include <vtkXMLDataElement.h>
#include <mitkAction.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>


//##Documentation
//## this class builds up all the necessary structures for a statemachine.
//## and stores one startstates for all built statemachines.
//##
//##

mitk::StateMachineFactory::StartStateMap mitk::StateMachineFactory::m_StartStates;

//XML StateMachine
//##ModelId=3E7757280322
const std::string mitk::StateMachineFactory::STYLE = "STYLE";
//##ModelId=3E775728037F
const std::string mitk::StateMachineFactory::NAME = "NAME";
//##ModelId=3E77572803DD
const std::string mitk::StateMachineFactory::ID = "ID";	  
//##ModelId=3E7757290053
const std::string mitk::StateMachineFactory::START_STATE = "START_STATE";
//##ModelId=3E77572900B1
const std::string mitk::StateMachineFactory::NEXT_STATE_ID = "NEXT_STATE_ID";
//##ModelId=3E775729010E
const std::string mitk::StateMachineFactory::EVENT_ID = "EVENT_ID";
//##ModelId=3E775729017C
const std::string mitk::StateMachineFactory::SIDE_EFFECT_ID = "SIDE_EFFECT_ID";
//##ModelId=3E7F18FF0131
const std::string mitk::StateMachineFactory::ISTRUE = "TRUE";
//##ModelId=3E7F18FF01FD
const std::string mitk::StateMachineFactory::ISFALSE = "FALSE";

const std::string mitk::StateMachineFactory::STATE_MACHIN = "stateMachine";

const std::string mitk::StateMachineFactory::STATE = "state";

const std::string mitk::StateMachineFactory::TRANSITION = "transition";

const std::string mitk::StateMachineFactory::STATE_MACHIN_NAME = "stateMachine";

const std::string mitk::StateMachineFactory::ACTION = "action";

const std::string mitk::StateMachineFactory::BOOL_PARAMETER = "boolParameter";

const std::string mitk::StateMachineFactory::INT_PARAMETER = "intParameter";

const std::string mitk::StateMachineFactory::FLOAT_PARAMETER = "floatParameter";

const std::string mitk::StateMachineFactory::DOUBLE_PARAMETER = "doubleParameter";

const std::string mitk::StateMachineFactory::STRING_PARAMETER = "stringParameter";

const std::string mitk::StateMachineFactory::VALUE = "value";

//##ModelId=3E68B2C600BD
mitk::StateMachineFactory::StateMachineFactory()
: m_AktState(NULL), m_AktTransition(NULL),	m_AktAction(NULL), m_AktStateMachineName(""){}

//##ModelId=3E5B4144024F
//##Documentation
//## returns NULL if no entry with string type is found
mitk::State* mitk::StateMachineFactory::GetStartState(const char * type)
{
	StartStateMapIter tempState = m_StartStates.find(type);
	if( tempState != m_StartStates.end() )
        return (tempState)->second;
	else
		return NULL;
}

//##ModelId=3E5B41730261
//##Documentation
//##loads the xml file filename and generates the necessary instances
bool mitk::StateMachineFactory::LoadBehavior(std::string fileName)
{
   if ( fileName.empty() )
       return false;

   mitk::StateMachineFactory* stateMachineFactory = new StateMachineFactory();
   stateMachineFactory->SetFileName( fileName.c_str() );

   if ( stateMachineFactory->Parse() )    
   {
     mitk::StatusBar::DisplayErrorText( "No appropriate statemachine found! Check string in constructor of interactor!" );
   }
  
   stateMachineFactory->Delete();
   return true;
}

//##ModelId=3E77572A010E
//##Documentation
//## recusive method, that parses this brand of 
//## the stateMachine; if the history has the same 
//## size at the end, then the StateMachine is correct
bool mitk::StateMachineFactory::parse(mitk::State::StateMap *states, mitk::State::StateMapIter thisState, HistorySet *history)
{
	history->insert((thisState->second)->GetId());//log our path  //or thisState->first
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
    StatusBar::DisplayText("Warnung: Ein inkonsistenter Zustand (oder ein Endzustand) wird erzeugt!");    
	  return true;//Jedoch erlaubt!!!z.B. als Endzustand
	}
	bool ok;
	//go through all Transitions of thisState and look if we have allready been in this state
	for (std::set<int>::iterator i = nextStatesSet.begin(); i != nextStatesSet.end();  i++)
	{
	  if ( history->find(*i) == history->end() )//wenn wir noch nicht in dieser NextState waren
	  {
	    mitk::State::StateMapIter nextState = states->find(*i);//search the iterator for our nextState
      if (nextState == states->end())
      {
        std::cout<<std::endl<<"Didn't find a state in StateMap!Check your statemachine behavior file!"<<std::endl;
        //you don't really see the warning in output! itkWarningMacro not possible due to this pointer!
        ok = false;
      }
      else
  		  ok = parse( states, nextState, history);//recusive path into the next state
    }
	}
	return ok;
}

//##ModelId=3E5B428F010B
//##Documentation
//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
bool mitk::StateMachineFactory::ConnectStates(mitk::State::StateMap *states)
{
    if (states->size() > 1)//only one state; don't have to be parsed for deadlocks!
	{
	  //parse all the given states an check for deadlock or not connected states
	  HistorySet *history = new HistorySet;
	  mitk::State::StateMapIter firstState = states->begin(); 
	  //parse through all the given states, log the parsed elements in history
	  bool ok = parse( states, firstState, history);

        
	  if ( (states->size() == history->size()) && ok )
	  {
	    delete history;
	  }
	  else //ether !ok or sizeA!=sizeB
	  {
	    delete history;
        mitk::StatusBar::DisplayText("Warnung: Ein unereichbarer Zustand wird aufgebaut. Ueberprüfen sie die Zustands- Konfigurations- Datei");    
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
       		mitk::StatusBar::DisplayText("Warnung: Das Verbinden der Zustände war NICHT erfolgreich!");    
            return false;//abort!
		}
	}
	return true;
}

//##ModelId=3E6773790098
void  mitk::StateMachineFactory::StartElement (const char *elementName, const char **atts) {

  std::string name(elementName);

	if ( name == STATE_MACHIN ) 					
  { 
    m_AktStateMachineName = ReadXMLStringAttribut( NAME, atts ); 
  } 
  
  else if ( name == STATE )   
  {
    std::string stateMachinName = ReadXMLStringAttribut( NAME, atts ) ;
    int id = ReadXMLIntegerAttribut( ID, atts );

		m_AktState = new mitk::State(stateMachinName , id);
		std::pair<mitk::State::StateMapIter,bool> ok = m_AllStates.insert(mitk::State::StateMap::value_type(id , m_AktState));

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
    m_AktTransition = new Transition(transitionName, nextStateId, eventId);

    if ( m_AktState )
		  m_AktState->AddTransition( m_AktTransition );  
  }

  else if ( name == ACTION )
  {

    int actionId = ReadXMLIntegerAttribut( ID, atts );
    m_AktAction = new Action( actionId );
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

//##
void mitk::StateMachineFactory::EndElement (const char *elementName) 
{
  bool ok = true;
  std::string name(elementName);

  if ( name == STATE_MACHIN_NAME )
	{
			ok = ConnectStates(&m_AllStates);
			m_AllStates.clear();
  } 
  else if ( name == STATE_MACHIN ) 
  {
    // m_AktState = NULL;
    // m_StartStates = NULL;
  } 
  else if ( name == TRANSITION ) 
  {
    m_AktTransition = NULL;
  } else if ( name == ACTION ) 
  {
    m_AktAction = NULL;
  }
}

//##
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

//##
int mitk::StateMachineFactory::ReadXMLIntegerAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );
  return atoi( s.c_str() );
}

//##
float mitk::StateMachineFactory::ReadXMLFloatAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );
  return (float) atof( s.c_str() );
}

//##
double mitk::StateMachineFactory::ReadXMLDoubleAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );
  return atof( s.c_str() );
}

//##
bool mitk::StateMachineFactory::ReadXMLBooleanAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );

  if ( s == ISTRUE )
    return true;
  else
    return false;
}
