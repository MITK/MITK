/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
