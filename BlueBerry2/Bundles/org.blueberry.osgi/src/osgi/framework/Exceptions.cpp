/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "Exceptions.h"

#include <typeinfo>

namespace osgi
{

namespace framework
{

POCO_IMPLEMENT_EXCEPTION(BadWeakPointerException, Poco::Exception, "Bad WeakPointer exception")

POCO_IMPLEMENT_EXCEPTION(IllegalArgumentException, Poco::LogicException, "Illegal argument")

POCO_IMPLEMENT_EXCEPTION(UnsupportedOperationException, Poco::RuntimeException, "Unsupported operation")

POCO_IMPLEMENT_EXCEPTION(IllegalStateException, Poco::RuntimeException, "Illegal state exception")

POCO_IMPLEMENT_EXCEPTION(PlatformException, Poco::RuntimeException, "Platform exception")

POCO_IMPLEMENT_EXCEPTION(ServiceException, Poco::RuntimeException, "Service exception")

POCO_IMPLEMENT_EXCEPTION(ManifestException, PlatformException, "Manifest syntax error")

POCO_IMPLEMENT_EXCEPTION(BundleException, PlatformException, "Bundle error")

POCO_IMPLEMENT_EXCEPTION(BundleStateException, PlatformException, "Bundle state invalid")

POCO_IMPLEMENT_EXCEPTION(BundleVersionConflictException, PlatformException, "Bundle version conflict")

POCO_IMPLEMENT_EXCEPTION(BundleLoadException, PlatformException, "Bundle load error")

POCO_IMPLEMENT_EXCEPTION(BundleResolveException, PlatformException, "Cannot resolve bundle")

POCO_IMPLEMENT_EXCEPTION(CoreException, PlatformException, "Core exception")

POCO_IMPLEMENT_EXCEPTION(InvalidServiceObjectException, CoreException, "Invalid service object")

InvalidSyntaxException::InvalidSyntaxException(const std::string& msg,
const std::string& filter) :
Poco::LogicException(msg), filter(filter)
{
}

InvalidSyntaxException::InvalidSyntaxException(const std::string& msg,
const std::string& filter, const Poco::Exception& exc) :
Poco::LogicException(msg, exc), filter(filter)
{
}

InvalidSyntaxException::InvalidSyntaxException(
const InvalidSyntaxException& exc) :
Poco::LogicException(exc), filter(exc.filter)
{
}

InvalidSyntaxException::~InvalidSyntaxException() throw ()
{
}

InvalidSyntaxException& InvalidSyntaxException::operator =(
const InvalidSyntaxException& exc)
{
Poco::LogicException::operator =(exc);
filter = exc.filter;
return *this;
}

const char* InvalidSyntaxException::name() const throw ()
{
return "Invalid syntax";
}

const char* InvalidSyntaxException::className() const throw ()
{
return typeid(*this).name();
}

Poco::Exception* InvalidSyntaxException::clone() const
{
return new InvalidSyntaxException(*this);
}

void InvalidSyntaxException::rethrow() const
{
throw *this;
}

std::string InvalidSyntaxException::getFilter() const throw ()
{
return filter;
}

const int ServiceExceptionCode::UNSPECIFIED = 0;
const int ServiceExceptionCode::UNREGISTERED = 1;
const int ServiceExceptionCode::FACTORY_ERROR = 2;
const int ServiceExceptionCode::FACTORY_EXCEPTION = 3;
const int ServiceExceptionCode::SUBCLASSED = 4;
const int ServiceExceptionCode::REMOTE = 5;

const int StatusExceptionCode::CODE_OK = 0x01;
const int StatusExceptionCode::CODE_INFO = 0x02;
const int StatusExceptionCode::CODE_WARNING = 0x04;
const int StatusExceptionCode::CODE_ERROR = 0x08;

}
}
