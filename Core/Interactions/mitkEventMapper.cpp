#include "EventMapper.h"

//XML Event	  
//##ModelId=3E788FC000E5
const std::string mitk::EventMapper::STYLE = "STYLE";
//##ModelId=3E788FC0025C
const std::string mitk::EventMapper::NAME = "NAME";
//##ModelId=3E788FC002AA
const std::string mitk::EventMapper::ID = "ID";
//##ModelId=3E77572901E9
const std::string mitk::EventMapper::TYPE = "TYPE";
//##ModelId=3E7757290256
const std::string mitk::EventMapper::BUTTON_NUMBER = "BUTTON_NUMBER";
//##ModelId=3E77572902C4
const std::string mitk::EventMapper::KEY = "KEY";
//##ModelId=3E7757290341
const std::string mitk::EventMapper::TRUE = "TRUE";
//##ModelId=3E77572903AE
const std::string mitk::EventMapper::FALSE = "FALSE";


//##ModelId=3E5B349600CB
mitk::EventMapper::SetStateMachine(StateMachine* stateMachine)
{
	m_StateMachine = stateMachine;
}

//##ModelId=3E5B34CF0041
bool mitk::EventMapper::SetEvent(Event* event)
{
}

//##ModelId=3E5B35140072
bool mitk::EventMapper::LoadBehavior(std::string fileName)
{
	  if ( fileName.empty() )
       return false;

   QFile xmlFile( fileName.c_str() );
   if ( !xmlFile.exists() )
       return false;

   QXmlInputSource source( &xmlFile );
   QXmlSimpleReader reader;
   mitk::EventMapper* eventMapper = new EventMapper();
   reader.setContentHandler( eventMapper );
   reader.parse( source );
  
   delete eventMapper;
   return true;
}

//##ModelId=3E788FC00308
bool mitk::EventMapper::startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts )
{
	/*
	qName.ascii(); //for debuging
	if( qName == "events")
	{
		std::string name = atts.value( STYLE.c_str() ).latin1();//wohin speichern?
	}
	else if ( qName == "event")
	{
		//neuen Eintrag in der m_EventDescriptions
		EventDescription tempEventDescr( atts.value( NAME.c_str() ).latin1(), 
										 atts.value( ID.c_str() ).toInt(), 
										 atts.value( TYPE.c_str() ).toInt(),
										 atts.value( BUTTON_NUMBER.c_str() ).toInt(), 
										 atts.value( KEY.c_str() ).toInt() );
		StateEvent tempStateEvent;//was hier?????????????????????????????
		std::pair<EventDescriptionMapIter,bool> ok = 
			m_EventDescriptions.insert(EventDescriptionMap::value_type (tempEventDescr, tempStateEvent));
		return ok.second;
	}
	
	*/
	return true;
}

//##ModelId=3E788FC100B6
bool mitk::EventMapper::endElement( const QString&, const QString&, const QString & qName )
{
	return true;
}

