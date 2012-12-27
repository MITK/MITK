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

