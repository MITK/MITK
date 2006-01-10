#ifndef MITK_CHILI_CONFERENCE_KIT_FACTORY_H
#define MITK_CHILI_CONFERENCE_KIT_FACTORY_H

#include <mitkConferenceKit.h>
#include <mitkConferenceKitFactory.h>

namespace mitk{

class ChiliConferenceKitFactory: public ConferenceKitFactory {
public:
  ChiliConferenceKitFactory();
  virtual ~ChiliConferenceKitFactory();

  //ConferenceKitFactory::Pointer GetInstance();
  
  virtual ConferenceKit::Pointer GetConferenceKit();

private:
  static ConferenceKit::Pointer m_QCInstance;

protected:
  

};

}//namespace

#endif
