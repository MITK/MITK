#include "StateMachineFactory.h"

//##ModelId=3E5B4144024F
//##Documentation
//## returns NULL if no entry with string type is found
State* mitk::StateMachineFactory::GetStartState(std::string type)
{
	State *tempState = m_StartStates.find(type);
	if( &tempStates != m_StartStates.end() )
        return tempState;
	else
		return NULL;
}

//##ModelId=3E5B41730261
//##Documentation
//##loads the xml file filename and generates the necessary instances
bool mitk::StateMachineFactory::LoadBehavior(std::string fileName)
{
   if ( fileName.isEmpty() )
       return false;

   QFile xmlFile( fileName );
   if ( !xmlFile.exists() )
       return false;

   QXmlInputSource source( &xmlFile );
   QXmlSimpleReader reader;
   reader.setContentHandler( this );
   reader.parse( source );
   return true;
}

//##ModelId=3E5B428F010B
//##Documentation
//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
bool mitk::StateMachineFactory::ConnectStates(vector<State*> states)
{
}


//##ModelId=3E6773790098
bool mitk::StateMachineFactory::StartElement( const QString&, const QString&, const QString& , const QXmlAttributes& )
{
	if ( qName == "mitkInteraktionStates" ) //new statemachine pattern, e.g.<mitkInteraktionStates STYLE="Powerpoint">
		NULL;
	else if ( qName == "stateMaschine" )// new statemachine global/ local... e.g. <stateMaschine NAME="global">
		NULL;
	else if ( qName == "state" )//new state, e.g. <state NAME="start" ID="1" START_STATE="TRUE">
		NULL;
	else if ( qName == "transition" )//new transition, e.g. <transition NAME="neues Element" NEXT_STATE_ID="2" EVENT="1" SIDE_EFFECT="0" />
		NULL;
	else if ( qName == "events" )//new events pattern, e.g. <events STYLE="PowerPoint">
		NULL;
	else if ( qName == "event" )//new events, e.g. <event NAME="Einfügen" ID="1" TYPE="1" BUTTON_NUMBER="1" KEY="56" />
		NULL;
	else
		NULL;

    return true;
}

//##ModelId=3E67737901E0
bool mitk::StateMachineFactory::EndElement( const QString&, const QString&, const QString& )
{
}