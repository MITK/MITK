#ifndef EVENTMAPPER_H_HEADER_INCLUDED_C187864A
#define EVENTMAPPER_H_HEADER_INCLUDED_C187864A

#include "mitkEvent.h"
#include "mitkCommon.h"
#include "mitkEventDescription.h"
#include "mitkStateEvent.h"
#include "mitkStateMachine.h"
#include <qxml.h>
#include <vector>
#include <string>
#include <map>

namespace mitk {
struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

//##ModelId=3E5A390401F2
//##Documentation
//## @brief Maps an Event to its description 
//## @ingroup Interaction
//## 
//## EventMapping:
//## This class mapps the Events, usually given by the OS or here by QT, to a MITK internal EventId.
//## It loads all information from the xml-file (possible, understandable Events with the mitkEventID).
//## If an event appears, the method MapEvent is called with the event params.
//## This Method looks up the event params, and tries to find an mitkEventId to it.
//## If yes, then sends the event and the found ID to the globalStateMachine, which handles all 
//## further operations of that event.
//## For Undo-Mechanism a statechanging StateMachine::HandleEvent is connected to an ObjectEventID and an GroupEventId.
//## That way a fine an raw Undo is possible (fine for ObjectID by ObjectID, raw for GroupID for GroupID)
//## Here the ObjectEventID gets increased, 
//## not the GroupEventId(must get increased by a StateMachine, that has the information when a new Group of operation starts)
class EventMapper : public QXmlDefaultHandler
{
  public:
  //##ModelId=3F02F896006D
  EventMapper();
  //##ModelId=3F02F896008C
  ~EventMapper();

	//##ModelId=3F0177080269
	typedef std::vector<mitk::EventDescription> EventDescriptionVec;
	//##ModelId=3F0177080288
	typedef std::vector<mitk::EventDescription>::iterator EventDescriptionVecIter;

  typedef std::map<const char*, int, ltstr> ConstMap;
  typedef std::map<const char*, int, ltstr>::iterator ConstMapIter;

  //##ModelId=3E5B349600CB
  //##Documentation
	//## set the global StateMachine. If temporaryly changed,
	//## then copy the old statemachine with GetStateMachine()
  static void SetGlobalStateMachine(StateMachine* stateMachine);

  //##ModelId=3EDCA746026F
	static StateMachine* GetGlobalStateMachine();

  //##ModelId=3E5B34CF0041
	//##Documentation
	//## searches the Event in m_EventDescription
	//## and if included
  static bool MapEvent(Event* event);

  //##ModelId=3E5B35140072
	//##Documentation
	//## loads an XML-File containing Events into m_EventDescriptions
	//## also involved: EventMapper::startEvent(...)
	static bool LoadBehavior(std::string fileName);

  //##ModelId=3E788FC00308
	//##Documentation
	//## reads a Tag from an XML-file
	//## adds Events to m_EventDescription
	bool startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts );

	//##ModelId=3E7B20EE01F5
  std::string GetStyleName();


  private:
  //##Documentation
  //## @brief converts the strings given by the XML-Behaviour-File to int
  inline const int convertConstString2ConstInt(std::string input);

  //##Documentation
  //## @brief maps the strings to int for convertion from XML-Behaviour-File
  ConstMap m_EventConstMap;

  //##ModelId=3E5B33F303CA
  //##Documentation
  //## @brief stores the information for the connection between QT-Events and the internal EventId.
  //## gets this information from xml-File
	static EventDescriptionVec m_EventDescriptions;

  //##ModelId=3E5B343701F1
  static StateMachine *m_GlobalStateMachine;

	//##ModelId=3E956E3A036B
  static StateEvent m_StateEvent;

	//##ModelId=3E7B1EB800CC
  //##Documentation
  //## @brief stores the name of the Event-Style loaded
  static std::string m_StyleName;

  //##ModelId=3E788FC000E5
	static const std::string STYLE;
  //##ModelId=3E788FC0025C
	static const std::string NAME;
  //##ModelId=3E788FC002AA
	static const std::string ID;
  //##ModelId=3E785B1B00FD
	static const std::string TYPE;
  //##ModelId=3E785B1B015B
	static const std::string BUTTON;
  //##ModelId=3E8B08FA01AA
  static const std::string BUTTONSTATE;
  //##ModelId=3E785B1B01A9
	static const std::string KEY;
};
} // namespace mitk

#endif /* EVENTMAPPER_H_HEADER_INCLUDED_C187864A */

