/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExceptionMacro_h
#define mitkExceptionMacro_h

#include <mitkException.h>
#include <itkMacro.h>
#include <mitkLog.h>
#include <sstream>

/**
 * \brief Throw a generic mitk::Exception with source location information.
 *
 * Use the stream operator to append a human-readable message:
 * \code
 * mitkThrow() << "this is error info";
 * \endcode
 *
 * \sa mitk::Exception, mitkReThrow, mitkThrowException
 */
#define mitkThrow() throw mitk::Exception(__FILE__, __LINE__, "", ITK_LOCATION)

/**
 * \brief Rethrow an existing mitk::Exception while recording rethrow context.
 *
 * The file and line of the rethrow site are stored in the exception so that
 * the full rethrow chain can be inspected via mitk::Exception::GetNumberOfRethrows()
 * and mitk::Exception::GetRethrowData().
 *
 * Example:
 * \code
 * try
 * {
 *   // some code that throws an exception
 * }
 * catch (mitk::Exception &e)
 * {
 *   mitkReThrow(e) << "Message appended to the exception (optional)";
 * }
 * \endcode
 *
 * \param mitkexception The mitk::Exception (or derived) object to rethrow.
 *
 * \sa mitk::Exception::AddRethrowData
 */
#define mitkReThrow(mitkexception)                                                                                     \
  mitkexception.AddRethrowData(__FILE__, __LINE__, "Rethrow by mitkReThrow macro.");                                   \
  throw mitkexception

/**
 * \brief Throw a specialized exception derived from mitk::Exception.
 *
 * The \p classname must be a class that inherits from mitk::Exception and uses
 * the mitkExceptionClassMacro in its definition. Use the stream operator to
 * append an error message:
 * \code
 * mitkThrowException(MySpecializedException) << "this is error info";
 * \endcode
 *
 * \param classname Fully qualified class name of the exception to throw.
 *
 * \sa mitk::Exception, mitkExceptionClassMacro
 */
#define mitkThrowException(classname) throw classname(__FILE__, __LINE__, "", ITK_LOCATION)

/**
 * \brief Convenience macro for defining MITK exception subclasses.
 *
 * Generates a constructor compatible with the exception macros and provides
 * stream operators and ITK RTTI support. All MITK exception classes should
 * derive from mitk::Exception and use this macro in their public section.
 *
 * \param ClassName      Name of the new exception class.
 * \param SuperClassName Name of the parent exception class (typically mitk::Exception).
 *
 * \sa mitk::Exception, mitkThrowException
 */
#define mitkExceptionClassMacro(ClassName, SuperClassName)                                                             \
  ClassName(const char *file, unsigned int lineNumber, const char *desc, const char *loc)                              \
    : SuperClassName(file, lineNumber, desc, loc)                                                                      \
  {                                                                                                                    \
  }                                                                                                                    \
  itkTypeMacro(ClassName, SuperClassName);                                                                             \
  /** \brief Definition of the bit shift operator for this class. It can be used to add messages.*/                    \
  template <class T>                                                                                                   \
  inline ClassName &operator<<(const T &data)                                                                          \
  {                                                                                                                    \
    std::stringstream ss;                                                                                              \
    ss << this->GetDescription() << data;                                                                              \
    this->SetDescription(ss.str());                                                                                    \
    return *this;                                                                                                      \
  }                                                                                                                    \
  /** \brief Definition of the bit shift operator for this class (for non const data).*/                               \
  template <class T>                                                                                                   \
  inline ClassName &operator<<(T &data)                                                                                \
  {                                                                                                                    \
    std::stringstream ss;                                                                                              \
    ss << this->GetDescription() << data;                                                                              \
    this->SetDescription(ss.str());                                                                                    \
    return *this;                                                                                                      \
  }                                                                                                                    \
  /** \brief Definition of the bit shift operator for this class (for functions).*/                                    \
  inline ClassName &operator<<(std::ostream &(*func)(std::ostream &))                                                  \
  {                                                                                                                    \
    std::stringstream ss;                                                                                              \
    ss << this->GetDescription() << func;                                                                              \
    this->SetDescription(ss.str());                                                                                    \
    return *this;                                                                                                      \
  }

#endif
