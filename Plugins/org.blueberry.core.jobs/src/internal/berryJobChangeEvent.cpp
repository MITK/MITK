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

#include "berryJobChangeEvent.h"
#include "berryJob.h"

namespace berry
{

JobChangeEvent::JobChangeEvent() :
  delay(-1), reschedule(false)
{
}

Poco::Timestamp::TimeDiff JobChangeEvent::GetDelay() const
{
  return delay;
}

Job::Pointer JobChangeEvent::GetJob() const
{
  return job;
}

IStatus::Pointer JobChangeEvent::GetResult() const
{
  return result;
}

}
