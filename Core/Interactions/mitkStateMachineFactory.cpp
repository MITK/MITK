#include "StateMachineFactory.h"
#include "mitkStatusBar.h"


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



//##ModelId=3E68B2C600BD
mitk::StateMachineFactory::StateMachineFactory()
: m_AktState(NULL), 	m_AktStateMachineName(""){}

//##ModelId=3E5B4144024F
//##Documentation
//## returns NULL if no entry with string type is found
mitk::State* mitk::StateMachineFactory::GetStartState(std::string type)
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

   QFile xmlFile( fileName.c_str() );
   if ( !xmlFile.exists() )
       return false;

   QXmlInputSource source( &xmlFile );
   QXmlSimpleReader reader;
   mitk::StateMachineFactory* stateMachineFactory = new StateMachineFactory();
   reader.setContentHandler( stateMachineFactory );//this???
   reader.parse( source );
  
   delete stateMachineFactory;
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
      (StatusBar::GetInstance())->DisplayText("Warnung: Ein inkonsistenter Zustand (oder ein Endzustand) wird erzeugt!");    
	  return true;//Jedoch erlaubt!!!z.B. als Endzustand
	}
	bool ok;
	//go through all Transitions of thisState and look if we have allready been in this state
	for (std::set<int>::iterator i = nextStatesSet.begin(); i != nextStatesSet.end();  i++)
	{
	  if ( history->find(*i) == history->end() )//wenn wir noch nicht in dieser NextState waren
	  {
	    mitk::State::StateMapIter nextState = states->find(*i);//search the iterator for our nextState
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
        (StatusBar::GetInstance())->DisplayText("Warnung: Ein unereichbarer Zustand wird aufgebaut. Ueberprüfen sie die Zustands- Konfigurations- Datei");    
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
       		(StatusBar::GetInstance())->DisplayText("Warnung: Das Verbinden der Zustände war NICHT erfolgreich!");    
            return false;//abort!
		}
	}
	return true;
}


//##ModelId=3E6773790098
bool mitk::StateMachineFactory::startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts )
{

	qName.ascii(); //for debuging
	
	//abfangen atts.value(#) fehler!
/*
	if( qName == "mitkInteraktionStates" )								//e.g.<mitkInteraktionStates STYLE="Powerpoint">
	{
		NULL;//new statemachine pattern
		//evtl. unterschied in STYLES PowerPoint oder PhotoShop usw...
	}

	else*/ if ( qName == "stateMachine" ) 									//e.g. <stateMachine NAME="global">
	{
		m_AktStateMachineName = atts.value( NAME.c_str() ).latin1() ;
	}

	else if ( qName == "state" )											//e.g. <state NAME="start" ID="1" START_STATE="ISTRUE">
	{
		m_AktState = new mitk::State( atts.value ( NAME.c_str() ).latin1(), ( atts.value ( ID.c_str() ) ).toInt() );
		std::pair<mitk::State::StateMapIter,bool> ok = m_AllStates.insert( mitk::State::StateMap::value_type( ( atts.value( ID.c_str() ) ).toInt(), m_AktState ) );
		//insert into m_AllStates, which stores all States that aren't connected yet!
		if (ok.second == false) 
			return false;//STATE_ID was not unique or something else didn't work in insert! EXITS the process
		if ( atts.value( START_STATE.c_str() ) == ISTRUE.c_str() )
			m_StartStates.insert(StartStateMap::value_type(m_AktStateMachineName, m_AktState));
			//if it is a startstate, then set a pointer into m_StartStates
	}

	else if ( qName == "transition" )										//e.g. <transition NAME="neues Element" NEXT_STATE_ID="2" EVENT="1" SIDE_EFFECT="0" />
	{
//		int a = atts.value( NEXT_STATE_ID.c_str() ).toInt(); //for debugging

		m_AktState->AddTransition( 
					atts.value(NAME.c_str()).latin1(), 
					atts.value(NEXT_STATE_ID.c_str()).toInt(),
					atts.value(EVENT_ID.c_str()).toInt(),
					atts.value(SIDE_EFFECT_ID.c_str()).toInt() );
	}
	/*else
		NULL;*/
	
    return true;
}

//##ModelId=3E6907B40180
bool mitk::StateMachineFactory::endElement( const QString&, const QString&, const QString & qName )
{
	//abfangen atts.value(#) fehler!
    bool ok = true;
	qName.ascii();
    if (qName == "stateMachine")
	{
			ok = ConnectStates(&m_AllStates);
			m_AllStates.clear();
	}
	return ok;
}
