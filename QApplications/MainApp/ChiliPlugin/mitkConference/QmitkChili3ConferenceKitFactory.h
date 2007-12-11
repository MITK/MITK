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

#ifndef MITK_CHILI_CONFERENCE_KIT_FACTORY_H
#define MITK_CHILI_CONFERENCE_KIT_FACTORY_H

#include <mitkConferenceKitFactory.h>


class Chili3ConferenceKitFactory: public mitk::ConferenceKitFactory {
  public:
    Chili3ConferenceKitFactory();
    virtual ~Chili3ConferenceKitFactory();

    virtual mitk::ConferenceKit* CreateConferenceKit();

};


#endif
