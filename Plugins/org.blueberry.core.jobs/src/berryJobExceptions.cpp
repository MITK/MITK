/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryJobExceptions.h"

#include <typeinfo>

namespace berry
{

POCO_IMPLEMENT_EXCEPTION(InterruptedException, Poco::RuntimeException, "Interrupted Exception")
POCO_IMPLEMENT_EXCEPTION(JobRuntimeException, Poco::RuntimeException, "Job Runtime Exception")
POCO_IMPLEMENT_EXCEPTION(IllegalStateException, Poco::RuntimeException, "IllegalState")
POCO_IMPLEMENT_EXCEPTION(FinallyThrowException, Poco::RuntimeException, "FinallyThrow")
POCO_IMPLEMENT_EXCEPTION(IllegalArgumentException, Poco::Exception, "IllegalArgument")

}

