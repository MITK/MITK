/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryPlatformLogChannel.h"

#include "cherryPlatform.h"
#include "../event/cherryPlatformEvent.h"

#include "Poco/Any.h"

namespace cherry {

PlatformLogChannel::PlatformLogChannel(const std::string& path)
 : Poco::SimpleFileChannel(path)
{
  
}
  
void 
PlatformLogChannel::log(const Poco::Message& msg)
{
  Poco::SimpleFileChannel::log(msg);
  
  PlatformEvent event(PlatformEvent::EV_LOGGED);
  Poco::Any data(msg);
  event.SetData(&data);
  
  Platform::GetEvents().logged(event);
}

}
