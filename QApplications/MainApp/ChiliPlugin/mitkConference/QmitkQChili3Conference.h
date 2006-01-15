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

    itkNewMacro(Self);

    //static mitk::ConferenceKit::Pointer QChili3Conference::GetInstance();
    QChili3Conference();
    ~QChili3Conference();

    void Launch();
    void SendQt(const char* s);
    void SendMITK(int eventID, short int p0, short int p1, short int p2);

  protected:
    //QChili3Conference();


};

#endif
