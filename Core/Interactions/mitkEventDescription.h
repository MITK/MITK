#ifndef EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D
#define EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D

//#include "mitkCommon.h"
#include "Event.h"
#include <string>

namespace mitk {

//##ModelId=3E5A3910009B
class EventDescription : public Event
{
  public:
    //##ModelId=3E5B30A30095
	EventDescription(std::string name, int id, int type, int buttonNumber, int key);

    //##ModelId=3E5B3103030A
	std::string GetName() const;

    //##ModelId=3E5B3132027C
    int GetId() const;

  private:
    //##ModelId=3E5B2F66020C
	std::string m_Name;

    //##ModelId=3E5B2F660220
    int m_Id;

};

} // namespace mitk



#endif /* EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D */
