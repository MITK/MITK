#include "StateMachineFactory.h"

//##Documentation
//## this class builds up all the necessary structures for a statemachine.
//## and stores one startstates for all built statemachines.
//##
//##

StartStateMap mitk::StateMachineFactory::m_StartStates;

//XML StateMachine
//##ModelId=3E72243D031F
const std::string mitk::StateMachineFactory::STYLE = "STYLE";
//##ModelId=3E72243D033E
const std::string mitk::StateMachineFactory::NAME = "NAME";
//##ModelId=3E72243D036D
const std::string mitk::StateMachineFactory::ID = "ID";	  
//##ModelId=3E72243D038C
const std::string mitk::StateMachineFactory::START_STATE = "START_STATE";
//##ModelId=3E72243D03BB
const std::string mitk::StateMachineFactory::NEXT_STATE_ID = "NEXT_STATE_ID";
//##ModelId=3E72243E0002
const std::string mitk::StateMachineFactory::EVENT_ID = "EVENT_ID";
//##ModelId=3E72243E0031
const std::string mitk::StateMachineFactory::SIDE_EFFECT_ID = "SIDE_EFFECT_ID";
//XML Event	  
//##ModelId=3E72243E005F
const std::string mitk::StateMachineFactory::TYPE = "TYPE";
//##ModelId=3E72243E009E
const std::string mitk::StateMachineFactory::BUTTON_NUMBER = "BUTTON_NUMBER";
//##ModelId=3E72243E00CD
const std::string mitk::StateMachineFactory::KEY = "KEY";
//##ModelId=3E72243E010B
const std::string mitk::StateMachineFactory::TRUE = "TRUE";
//##ModelId=3E72243E013A
const std::string mitk::StateMachineFactory::FALSE = "FALSE";


//##ModelId=3E68B2C600BD
mitk::StateMachineFactory::StateMachineFactory()
: m_AktState(NULL), 	m_AktStateMachineName(""){}

//##ModelId=3E72243E0188
//##Documentation
//## Destructor, deletes all the reserved memory
mitk::StateMachineFactory::~StateMachineFactory()
{
	
}


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
   reader.setContentHandler( stateMachineFactory );
   reader.parse( source );
  
   delete stateMachineFactory;
   return true;
}

//##Documentation
//## recusive method, that parses this brand of 
//## the stateMachine and returns if correct
/*bool parse(State *state, std::set *history)
{
	for (StateMapIter tempState = states->begin(); tempState != states->end();  tempState++)
	
}*/

//##ModelId=3E5B428F010B
//##Documentation
//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
bool mitk::StateMachineFactory::ConnectStates(StateMap *states)
{
	//parse all the given states an check for deadlock or not connected states
/*	std::set *history = new std::set;
	bool ok = parse(states->begin(), history);
	if !( (states->size() == history.size()) || ok )
	{
		delete history;
		return false;
	}
	else delete history;
	*/

	//connect all the states
	for (StateMapIter tempState = states->begin(); tempState != states->end();  tempState++)
	{
		//searched through the States and Connects all Transitions
		bool tempbool = ( ( tempState->second )->ConnectTransitions( states ) );
        if ( tempbool = false )
            return false;//abort!
	}
	return true;
}


//##ModelId=3E6773790098
bool mitk::StateMachineFactory::startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts )
{
	//qName.ascii(); //for debuging
	
	//abfangen atts.value(#) fehler!

	if( qName == "mitkInteraktionStates" )								//e.g.<mitkInteraktionStates STYLE="Powerpoint">
	{
		NULL;//new statemachine pattern
	}

	else if ( qName == "stateMaschine" ) 									//e.g. <stateMaschine NAME="global">
	{
		m_AktStateMachineName = atts.value( NAME.c_str() ).latin1() ;
	}

	else if ( qName == "state" )											//e.g. <state NAME="start" ID="1" START_STATE="TRUE">
	{
		m_AktState = new mitk::State( atts.value ( NAME.c_str() ).latin1(), ( atts.value ( ID.c_str() ) ).toInt() );
		std::pair<StateMapIter,bool> ok = m_AllStates.insert( StateMap::value_type( ( atts.value( ID.c_str() ) ).toInt(), m_AktState ) );
		//insert into m_AllStates, which stores all States that aren't connected yet!
		if (ok.second == false) 
			return false;//STATE_ID was not unique or something else didn't work in insert! EXITS the process
		if ( atts.value( START_STATE.c_str() ) == TRUE.c_str() )
			m_StartStates.insert(StartStateMap::value_type(m_AktStateMachineName, m_AktState));
			//if it is a startstate, then set a pointer into m_StartStates
	}

	else if ( qName == "transition" )										//e.g. <transition NAME="neues Element" NEXT_STATE_ID="2" EVENT="1" SIDE_EFFECT="0" />
	{
		int a = atts.value( NEXT_STATE_ID.c_str() ).toInt();

		m_AktState->AddTransition( 
					atts.value(NAME.c_str()).latin1(), 
					atts.value(NEXT_STATE_ID.c_str()).toInt(),
					atts.value(EVENT_ID.c_str()).toInt(),
					atts.value(SIDE_EFFECT_ID.c_str()).toInt() );
	}
	else if ( qName == "events" )											//new events pattern, e.g. <events STYLE="PowerPoint">
		//TODO: Events Loading!
	{NULL;}

	else if (qName == "event")											//new events, e.g. <event NAME="Einfügen" ID="1" TYPE="1" BUTTON_NUMBER="1" KEY="56" />
			//ToDo: Events Loading!
	{NULL;}

	else
		NULL;
	
    return true;
}

//##ModelId=3E6907B40180
bool mitk::StateMachineFactory::endElement( const QString&, const QString&, const QString & qName )
{
	//abfangen atts.value(#) fehler!

    if (qName == "stateMaschine")
	{
			bool ok = ConnectStates(&m_AllStates);
			m_AllStates.clear();
			if (!ok)
				return false;
	}

	return true;
}