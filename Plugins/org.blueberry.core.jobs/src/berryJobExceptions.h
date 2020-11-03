/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYJOBSEXCEPTIONS_H_
#define BERRYJOBSEXCEPTIONS_H_

#include <org_blueberry_core_jobs_Export.h>
#include <Poco/Exception.h>

namespace berry
{

POCO_DECLARE_EXCEPTION(BERRY_JOBS, InterruptedException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(BERRY_JOBS, IllegalStateException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(BERRY_JOBS, JobRuntimeException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(BERRY_JOBS, FinallyThrowException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(BERRY_JOBS, IllegalArgumentException, Poco::Exception)

}

#endif /* BERRYJOBSEXCEPTIONS_H_ */
