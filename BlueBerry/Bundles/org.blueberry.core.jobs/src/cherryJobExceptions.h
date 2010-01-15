/*=========================================================================

 Program:   openCherry Platform
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

#ifndef CHERRYJOBSEXCEPTIONS_H_
#define CHERRYJOBSEXCEPTIONS_H_

#include "cherryJobsDll.h"
#include <Poco/Exception.h>

namespace cherry
{

POCO_DECLARE_EXCEPTION(CHERRY_JOBS, InterruptedException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(CHERRY_JOBS, IllegalStateException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(CHERRY_JOBS, JobRuntimeException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(CHERRY_JOBS, FinallyThrowException, Poco::RuntimeException)
POCO_DECLARE_EXCEPTION(CHERRY_JOBS, IllegalArgumentException, Poco::Exception)

}

#endif /* CHERRYJOBSEXCEPTIONS_H_ */
