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

#ifndef BERRYPLATFORMLOGCHANNEL_H_
#define BERRYPLATFORMLOGCHANNEL_H_

#include "Poco/Message.h"
#include "Poco/SimpleFileChannel.h"

namespace berry {

class PlatformLogChannel : public Poco::SimpleFileChannel
{
public:
  PlatformLogChannel(const std::string& path);

  void log(const Poco::Message& msg);

};

}

#endif /*BERRYPLATFORMLOGCHANNEL_H_*/
