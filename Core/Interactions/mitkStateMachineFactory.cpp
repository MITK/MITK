#include "StateMachineFactory.h"

//##ModelId=3E5B4144024F
//##Documentation
//## returns NULL if no entry with string type is found
State* mitk::StateMachineFactory::GetStartState(std::string type)
{
	State *tempState = startStates.find(type);
	if( &tempStates != startStates.end() )
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
   QmitkXMLLoader handler(*startStates);
//...
   reader.setContentHandler( &handler );
   reader.parse( source );
   return true;
}



}

//##ModelId=3E5B428F010B
//##Documentation
//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
bool mitk::StateMachineFactory::ConnectStates(vector<State*> states)
{

}

