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

#ifndef MITK_QCHILI_CONFERENCE_H
#define MITK_QCHILI_CONFERENCE_H

#include <mitkConferenceKit.h>

class Chili3Conference : public mitk::ConferenceKit
{

  public:
    Chili3Conference();
    ~Chili3Conference();

    void Launch();
    void SendQt(const char* s);
    void SendMITK(signed int eventID, const char* sender, int etype, int estate, int ebuttonstate, int ekey, float w1, float w2, float w3, float p1, float p2);
    void MouseMove( const char* sender, float w1, float w2, float w3 );
    void MyTokenPriority(long int tid);
    void AskForToken(long int requester);
    void SetToken(long int sender, long int requester);


};

#endif

