#include "StateMachineFactory.h"

//##ModelId=3E68B2C600BD
mitk::StateMachineFactory::StateMachineFactory()
: m_AktState(NULL), 	m_AktStateMachineName("")
{
}

//##ModelId=3E5B4144024F
//##Documentation
//## returns NULL if no entry with string type is found
mitk::State* mitk::StateMachineFactory::GetStartState(std::string type)
{
/*	State *tempState = m_StartStates.find(type);
	if( &tempStates != m_StartStates.end() )
        return tempState;
	else
	####################################################################
	incorect!
	*/
		return NULL;
}

//##ModelId=3E5B41730261
//##Documentation
//##loads the xml file filename and generates the necessary instances
bool mitk::StateMachineFactory::LoadBehavior(std::string fileName)
{
   if ( fileName.empty() )
       return false;

/*   QFile xmlFile( ((QString)(fileName)) );
   if ( !xmlFile.exists() )
       return false;

   QXmlInputSource source( &xmlFile );
   QXmlSimpleReader reader;
   reader.setContentHandler( this );
   reader.parse( source );
   ################################################################
   incorect ! cannot convert std::string to QString
   */
   return true;
}

//##ModelId=3E5B428F010B
//##Documentation
//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
bool mitk::StateMachineFactory::ConnectStates(StateMap *states)
{
/*	iterator tempState = states->begin();
	for (int i = 0; i < states.size(); i++, tempState++)
	{
		bool tempbool = tempState->ConnectTransitions(states);//searched through the States and Connects all Transitions
        if (tempbool = false)
            return false;//abort!
	}
	#####################################
	incorect iterator
	*/
	return true;
}


//##ModelId=3E6773790098
bool mitk::StateMachineFactory::StartElement (const QString&, const QString&, const QString & qName, const QXmlAttributes & atts )
{
	//abfangen atts.value(#) fehler!
	if(qName == "mitkInteraktionStates")//e.g.<mitkInteraktionStates STYLE="Powerpoint">
            NULL;		//new statemachine pattern
	else if (qName == "stateMaschine")//e.g. <stateMaschine NAME="global">
		m_AktStateMachineName = atts.value(1);
	else if (qName == "state")//e.g. <state NAME="start" ID="1" START_STATE="TRUE">
	{
/*		m_AktState = new mitk::State(atts.value(1), atts.value(2));

		m_AllStates.insert(atts.value(2), m_AktState);
		if ((atts.lenth >2) && (atts.value(3) == "true"))//START_STATE     ###evtl. Fehlerquelle &&
			m_StartStates.insert(m_AktStateMachineName, m_AktState);
			##########################################################
			incorect, covertion problem qStdiung Std::sring
			*/
	}
	else if (qName == "transition")//e.g. <transition NAME="neues Element" NEXT_STATE_ID="2" EVENT="1" SIDE_EFFECT="0" />
		/*m_AktState->AddTransition(atts.value(1), atts.value(2), atts.value(3), atts.value(4));
		#############################################################
		incorect conversion problem qString std::sring
		*/
	{}
	else if (qName == "events")//new events pattern, e.g. <events STYLE="PowerPoint">
		//TODO: Events Loading!
	{}
	else if (qName == "event")//new events, e.g. <event NAME="Einfügen" ID="1" TYPE="1" BUTTON_NUMBER="1" KEY="56" />
			//ToDo: Events Loading!
	{}
	else
		NULL;
	
	
    return true;
}

//##ModelId=3E6907B40180
bool mitk::StateMachineFactory::EndElement( const QString&, const QString&, const QString & qName )
{
	//abfangen atts.value(#) fehler!

    if (qName == "stateMaschine")
	{
			bool tempbool = ConnectStates(&m_AllStates);
			m_AllStates.clear();
			if (tempbool = false)
				return false;
	}
	else
        NULL;
}