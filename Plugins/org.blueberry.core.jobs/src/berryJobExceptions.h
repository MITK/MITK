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
