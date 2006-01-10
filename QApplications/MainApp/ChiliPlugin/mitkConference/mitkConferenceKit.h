#ifndef MITK_CONFERENCE_KIT_H
#define MITK_CONFERENCE_KIT_H

#include "mitkConferenceToken.h"
#include <itkObject.h>

namespace mitk{
//class ConferenceKit:public ConferenceToken {
  class ConferenceKit:public itk::Object
  {
  public:
    typedef ConferenceKit  Self;
    typedef itk::SmartPointer<Self>   Pointer;
    typedef itk::SmartPointer<const Self>  ConstPointer;

 //   virtual void Launch() = 0;
 //   virtual void Close() = 0;
 //   virtual void UpdateMe() = 0;
 //   virtual void SendMITK() = 0;
    virtual void SendQt(const char* s) = 0;
    virtual ~ConferenceKit(){}
  };
}
#endif
