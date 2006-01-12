#ifndef MITK_CHILI_CONFERENCE_KIT_FACTORY_H
#define MITK_CHILI_CONFERENCE_KIT_FACTORY_H

//#include <mitkConferenceKit.h>
#include <mitkConferenceKitFactory.h>

namespace mitk{

class ChiliConferenceKitFactory: public ConferenceKitFactory {
public:
  ChiliConferenceKitFactory();
  virtual ~ChiliConferenceKitFactory();

  virtual ConferenceKit* CreateConferenceKit();

};

}//namespace

#endif
