#ifndef TEXT_LOGGER_CONFERENCE_KIT_FACTORY_H
#define TEXT_LOGGER_CONFERENCE_KIT_FACTORY_H

#include <mitkConferenceKitFactory.h>


class TextLoggerConferenceKitFactory: public mitk::ConferenceKitFactory {
  public:
    TextLoggerConferenceKitFactory();
    virtual ~TextLoggerConferenceKitFactory();

    virtual mitk::ConferenceKit* CreateConferenceKit();

};

#endif

