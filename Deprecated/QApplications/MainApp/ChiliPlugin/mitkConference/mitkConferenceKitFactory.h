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

#ifndef MITK_CONFERENCE_KIT_FACTORY_H
#define MITK_CONFERENCE_KIT_FACTORY_H

namespace mitk{

  class ConferenceKit;

  class ConferenceKitFactory {
    public:

      virtual ConferenceKit* CreateConferenceKit() = 0;
      virtual ~ConferenceKitFactory(){};

    protected:
      ConferenceKitFactory(){};
  };

}//namespace

#endif
