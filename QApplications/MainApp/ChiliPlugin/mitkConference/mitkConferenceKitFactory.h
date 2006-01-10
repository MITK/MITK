#ifndef MITK_CONFERENCE_KIT_FACTORY_H
#define MITK_CONFERENCE_KIT_FACTORY_H

#include <mitkConferenceKit.h>

namespace mitk{

class ConferenceKitFactory {
 public:

    static ConferenceKitFactory* GetInstance(int ClassKey = 0);
    virtual ConferenceKit::Pointer GetConferenceKit()=0;

    
  private:
    static ConferenceKitFactory* m_Instance;
  protected:
    
};

}//namespace

#endif
