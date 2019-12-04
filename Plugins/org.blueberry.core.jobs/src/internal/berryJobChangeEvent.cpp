/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
