#ifndef MITK_CHILI_CONFERENCE_KIT_FACTORY_H
#define MITK_CHILI_CONFERENCE_KIT_FACTORY_H

#include <mitkConferenceKitFactory.h>


class Chili3ConferenceKitFactory: public mitk::ConferenceKitFactory {
  public:
    Chili3ConferenceKitFactory();
    virtual ~Chili3ConferenceKitFactory();

    virtual mitk::ConferenceKit* CreateConferenceKit();

};


#endif
