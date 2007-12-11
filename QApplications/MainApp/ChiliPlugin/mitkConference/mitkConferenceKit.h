/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef MITK_CONFERENCE_KIT_H
#define MITK_CONFERENCE_KIT_H

#include "mitkConferenceToken.h"
#include "mitkConferenceKitFactory.h"
#include <itkObject.h>

namespace mitk{

  // Changes in the functions above has to follow a rounding to the next higher whole number
  // changes in Conference functions shall add +0.1 to significate a change but maybe not so dagnerouse for stability...
  const float ConferenceVersion = 0.1; 
  
  class ConferenceKitFactory;

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
    virtual void SendMITK(signed int,const char* sender, int, int, int, int, float, float, float, float, float) = 0;
    virtual void SendQt(const char* s) = 0;
    virtual void MouseMove( const char* sender,float, float, float ) = 0;
    virtual void MyTokenPriority(long int) = 0;
    virtual void AskForToken(long int requester) = 0;
    virtual void SetToken(long int sender,long int requester) = 0;
    virtual ~ConferenceKit(){}

  private:
    static ConferenceKit* m_Instance;
    static ConferenceKitFactory* m_ConferenceKitFactory;
  };
}
#endif
