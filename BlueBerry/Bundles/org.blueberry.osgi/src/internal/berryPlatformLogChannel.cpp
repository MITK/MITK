/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryPlatformLogChannel.h"

#include "berryPlatform.h"
#include "event/berryPlatformEvent.h"

#include "Poco/Any.h"

namespace berry {

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
