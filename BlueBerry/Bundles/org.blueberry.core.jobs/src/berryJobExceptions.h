/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of g Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef BERRYJOBSEXCEPTIONS_H_
#define BERRYJOBSEXCEPTIONS_H_

#include "berryJobsDll.h"
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
