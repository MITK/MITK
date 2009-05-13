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

#ifndef CONFERENCEVENTMAPPER_H_HEADER_INCLUDE
#define CONFERENCEVENTMAPPER_H_HEADER_INCLUDE

#include <mitkEventMapper.h>

namespace mitk {

  class MITKEXT_CORE_EXPORT ConferenceEventMapper : public EventMapper
  {
    public:
      ConferenceEventMapper();

      ~ConferenceEventMapper();

      // First part MITK Event ID and baserendere name
      // Second the normaly not needed integer information from the GUI Event (type, state, button, key)
      // Third the koordinated 3D and the relativ 2D
      static bool MapEvent(signed int id, const char* sender, int Etype, int Estate, int EButtonState, int key, float w1,float w2,float w3,float d0,float d1);

      /* EasyEvent for MousMove Interaction.
       * used during conferences
       */
      static bool MapEvent(const char* sender, float w1,float w2,float w3);
  };
} // namespace mitk

#endif


