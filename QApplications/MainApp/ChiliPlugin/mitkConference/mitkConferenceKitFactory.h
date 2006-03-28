#ifndef MITK_CONFERENCE_KIT_FACTORY_H
#define MITK_CONFERENCE_KIT_FACTORY_H

namespace mitk{

  class ConferenceKit;

  class ConferenceKitFactory {
    public:

      virtual ConferenceKit* CreateConferenceKit() = 0;
      virtual ~ConferenceKitFactory(){};

    protected:
      ConferenceKitFactory(){};
  };

}//namespace

#endif
