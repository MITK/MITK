
#ifndef MITK_QCHILI_CONFERENCE_H
#define MITK_QCHILI_CONFERENCE_H

#include <mitkConferenceKit.h>
#include <itkObjectFactory.h>

class QChili3Conference:public mitk::ConferenceKit
{

  public:

    typedef QChili3Conference Self;
    typedef itk::SmartPointer<Self>   Pointer;
    typedef itk::SmartPointer<const Self>  ConstPointer;

    mitkClassMacro(QChili3Conference,mitk::ConferenceKit);

    itkNewMacro(Self);

    //static mitk::ConferenceKit::Pointer QChili3Conference::GetInstance();
    QChili3Conference();
    ~QChili3Conference();

    void Launch();
    void SendQt(const char* s);
    void SendMITK(signed int eventID, const char* sender, float w1, float w2, float w3, float p1, float p2);

  protected:
    //QChili3Conference();


};

#endif


