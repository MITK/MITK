//
// FPEnvironment.cpp
//
// $Id$
//
// Library: Foundation
// Package: Core
// Module:  FPEnvironment
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


// pull in platform identification macros needed below
#include "Poco/Platform.h"


#if defined(POCO_NO_FPENVIRONMENT)
#include "FPEnvironment_DUMMY.cpp"
#elif defined(__osf__) || defined(__VMS)
#include "FPEnvironment_DEC.cpp"
#elif defined(sun) || defined(__sun)
#include "FPEnvironment_SUN.cpp"
#elif defined(POCO_OS_FAMILY_UNIX)
#include "FPEnvironment_C99.cpp"
#elif defined(POCO_OS_FAMILY_WINDOWS)
#include "FPEnvironment_WIN32.cpp"
#else
#include "FPEnvironment_DUMMY.cpp"
#endif


// header file must be included after platform-specific part
// due to _XOPEN_SOURCE conflict on Tru64 (see FPEnvironment_DEC.cpp)
#include "Poco/FPEnvironment.h"


namespace Poco {


FPEnvironment::FPEnvironment()
{
}


FPEnvironment::FPEnvironment(RoundingMode rm)
{
	setRoundingMode(rm);
}

	
FPEnvironment::FPEnvironment(const FPEnvironment& env): FPEnvironmentImpl(env)
{
}

	
FPEnvironment::~FPEnvironment()
{
}

	
FPEnvironment& FPEnvironment::operator = (const FPEnvironment& env)
{
	if (&env != this)
	{
		FPEnvironmentImpl::operator = (env);
	}
	return *this;
}


void FPEnvironment::keepCurrent()
{
	keepCurrentImpl();
}


void FPEnvironment::clearFlags()
{
	clearFlagsImpl();
}


} // namespace Poco
