#ifndef MITK_CONFERENCE_KIT_FACTORY_H
#define MITK_CONFERENCE_KIT_FACTORY_H

//#include <mitkConferenceKit.h>

namespace mitk{

class ConferenceKit;

class ConferenceKitFactory {
 public:

//    static ConferenceKitFactory* GetInstance(int ClassKey = 0) = 0;
    virtual ConferenceKit* CreateConferenceKit() = 0;
    virtual ~ConferenceKitFactory(){};

  private:
   // static ConferenceKitFactory* m_Instance;
  protected:
    ConferenceKitFactory(){};
};

}//namespace

#endif
