#ifndef EVENTMAPPER_H_HEADER_INCLUDED_C187864A
#define EVENTMAPPER_H_HEADER_INCLUDED_C187864A

//#include "mitkCommon.h"
#include "Event.h"
#include "EventDescription.h"
#include "StateEvent.h"
#include "StateMachine.h"
#include <qxml.h>
#include <vector>
#include <string>

//##ModelId=3E788FC0001A
typedef std::vector<mitk::EventDescription> EventDescriptionVec;
//##ModelId=3E788FC0002A
typedef std::vector<mitk::EventDescription>::iterator EventDescriptionVecIter;

namespace mitk {

//##ModelId=3E5A390401F2
class EventMapper : public QXmlDefaultHandler
{
  public:
    //##ModelId=3E5B349600CB
    static SetStateMachine(StateMachine* stateMachine);

    //##ModelId=3E5B34CF0041
    static bool MapEvent(Event* event);

    //##ModelId=3E5B35140072
	static bool LoadBehavior(std::string fileName);

    //##ModelId=3E788FC00308
	bool startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts );
    //##ModelId=3E7B20EE01F5
    std::string GetStyleName();


  private:
    //##ModelId=3E5B33F303CA
	static EventDescriptionVec m_EventDescriptions;

    //##ModelId=3E5B343701F1
    static StateMachine* m_StateMachine;
  
    //##ModelId=3E788FC000E5
	static const std::string STYLE;
    //##ModelId=3E788FC0025C
	static const std::string NAME;
    //##ModelId=3E788FC002AA
	static const std::string ID;
    //##ModelId=3E785B1B00FD
	static const std::string TYPE;
    //##ModelId=3E785B1B015B
	static const std::string BUTTON_NUMBER;
    //##ModelId=3E785B1B01A9
	static const std::string KEY;
    //##ModelId=3E785B1B01F7
	static const std::string TRUE;
    //##ModelId=3E785B1B0245
	static const std::string FALSE;
    //##ModelId=3E7B139E0233
    static StateEvent m_StateEvent;
    //##ModelId=3E7B1EB800CC
    static std::string m_StyleName;

};

} // namespace mitk



#endif /* EVENTMAPPER_H_HEADER_INCLUDED_C187864A */
