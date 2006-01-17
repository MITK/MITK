#ifndef MITK_CONFERENCE_KIT_H
#define MITK_CONFERENCE_KIT_H

#include "mitkConferenceToken.h"
#include "mitkConferenceKitFactory.h"
#include <itkObject.h>

namespace mitk{

class ConferenceKitFactory;

//class ConferenceKit:public ConferenceToken {
  class ConferenceKit:public itk::Object
  {
  public:
    typedef ConferenceKit  Self;
    typedef itk::SmartPointer<Self>   Pointer;
    typedef itk::SmartPointer<const Self>  ConstPointer;

    static void SetFactory( ConferenceKitFactory* factory );

    static ConferenceKit* GetInstance();

 //   virtual void Launch() = 0;
 //   virtual void Close() = 0;
 //   virtual void UpdateMe() = 0;
    virtual void SendMITK(signed int,const char* sender, float, float, float, float, float) = 0;
    virtual void SendQt(const char* s) = 0;
    virtual ~ConferenceKit(){}

  private:
    static ConferenceKit* m_Instance;
    static ConferenceKitFactory* m_ConferenceKitFactory;
  };
}
#endif
