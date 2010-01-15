/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef OSGI_FRAMEWORK_PLATFORMEXCEPTION_H_
#define OSGI_FRAMEWORK_PLATFORMEXCEPTION_H_

#include "../../cherryOSGiDll.h"
#include "Poco/Exception.h"

namespace osgi {

namespace framework {

struct StatusExceptionCode {

  /**
   * The exception is ok and expected
   */
  static const int CODE_OK; // = 0x01;
  /**
   * The exception is for informational purposes
   */
  static const int CODE_INFO; // = 0x02;
  /**
   * The exception is unexpected by may be handled, but a warning should be logged
   */
  static const int CODE_WARNING; // = 0x04;
  /**
   * The exception is unexpected and should result in an error.
   */
  static const int CODE_ERROR; // = 0x08;

};



POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BadWeakPointerException, Poco::Exception)

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, IllegalArgumentException, Poco::LogicException)

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, UnsupportedOperationException, Poco::RuntimeException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, IllegalStateException, Poco::RuntimeException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, PlatformException, Poco::RuntimeException);

/**
 * A service exception used to indicate that a service problem occurred.
 *
 * <p>
 * A <code>ServiceException</code> object is created by the Framework or
 * service implementation to denote an exception condition in the service. A
 * type code is used to identify the exception type for future extendability.
 * Service implementations may also create subclasses of
 * <code>ServiceException</code>. When subclassing, the subclass should set
 * the type to {@link #SUBCLASSED} to indicate that
 * <code>ServiceException</code> has been subclassed.
 *
 * <p>
 * This exception conforms to the general purpose exception chaining mechanism.
 *
 * @version $Revision$
 * @since 1.5
 */
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, ServiceException, Poco::RuntimeException);

struct ServiceExceptionCode {

  /**
   * No exception type is unspecified.
   */
  static const int UNSPECIFIED; //     = 0;
  /**
   * The service has been unregistered.
   */
  static const int UNREGISTERED; //    = 1;
  /**
   * The service factory produced an invalid service object.
   */
  static const int FACTORY_ERROR; //   = 2;
  /**
   * The service factory threw an exception.
   */
  static const int FACTORY_EXCEPTION; // = 3;
  /**
   * The exception is a subclass of ServiceException. The subclass should be
   * examined for the type of the exception.
   */
  static const int SUBCLASSED; //      = 4;
  /**
   * An error occurred invoking a remote service.
   */
  static const int REMOTE; //        = 5;

};

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, ManifestException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleStateException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleVersionConflictException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleLoadException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleResolveException, PlatformException);

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, CoreException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, InvalidServiceObjectException, CoreException);

/**
 * A Framework exception used to indicate that a filter string has an invalid
 * syntax.
 *
 * <p>
 * An <code>InvalidSyntaxException</code> object indicates that a filter
 * string parameter has an invalid syntax and cannot be parsed. See
 * {@link Filter} for a description of the filter string syntax.
 *
 */
class InvalidSyntaxException: public Poco::LogicException
  {
  public:
    InvalidSyntaxException(const std::string& msg, const std::string& filter);
    InvalidSyntaxException(const std::string& msg, const std::string& filter, const Poco::Exception& exc);
    InvalidSyntaxException(const InvalidSyntaxException& exc);
    ~InvalidSyntaxException() throw();
    InvalidSyntaxException& operator = (const InvalidSyntaxException& exc);
    const char* name() const throw();
    const char* className() const throw();
    Poco::Exception* clone() const;
    void rethrow() const;

    std::string getFilter() const throw();

  private:

    std::string filter;
  };

} }

#endif /*OSGI_FRAMEWORK_PLATFORMEXCEPTION_H_*/
