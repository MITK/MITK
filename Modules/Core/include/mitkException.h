/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkException_h
#define mitkException_h

#include <MitkCoreExports.h>
#include <itkMacro.h>
#include <vector>

namespace mitk
{
  /**
   * \brief Base class for all MITK exceptions.
   *
   * Extends itk::ExceptionObject with support for rethrow tracking and stream-style
   * message composition. Do not instantiate this class directly; use the exception
   * macros defined in mitkExceptionMacro.h instead.
   *
   * Simple usage:
   * \code
   * mitkThrow() << "optional exception message";
   * \endcode
   *
   * To define a specialized exception, inherit from this class and use the
   * mitkExceptionClassMacro:
   * \code
   * class MyException : public mitk::Exception
   * {
   * public:
   *   mitkExceptionClassMacro(MyException, mitk::Exception);
   * };
   * \endcode
   *
   * Throw specialized exceptions with:
   * \code
   * mitkThrowException(MyException) << "optional exception message";
   * \endcode
   *
   * \sa mitkThrow, mitkReThrow, mitkThrowException, mitkExceptionClassMacro
   * \ingroup Core
   */
  class MITKCORE_EXPORT Exception : public itk::ExceptionObject
  {
  public:
    /**
     * \brief Construct an exception with source location and description.
     * \param[in] file Source file where the exception was thrown.
     * \param[in] lineNumber Line number in the source file.
     * \param[in] desc Human-readable description of the error.
     * \param[in] loc Location string (typically the function name via ITK_LOCATION).
     */
    Exception(const char *file, unsigned int lineNumber = 0, const char *desc = "None", const char *loc = "Unknown")
      : itk::ExceptionObject(file, lineNumber, desc, loc)
    {
    }

    /** \brief Destructor. */
    ~Exception() throw() override {}
    itkTypeMacro(Exception, itk::ExceptionObject);

    /**
     * \brief Record additional rethrow context for this exception.
     *
     * Each call appends a new rethrow record. Use the mitkReThrow macro instead
     * of calling this directly.
     *
     * \param[in] file Source file where the rethrow occurred.
     * \param[in] lineNumber Line number of the rethrow.
     * \param[in] message Descriptive message for the rethrow.
     *
     * \sa mitkReThrow, GetNumberOfRethrows, GetRethrowData
     */
    void AddRethrowData(const char *file, unsigned int lineNumber, const char *message);

    /**
     * \brief Return the number of times this exception has been rethrown.
     * \return The rethrow count (0 if the exception was never rethrown).
     */
    int GetNumberOfRethrows();

    /**
     * \brief Retrieve the rethrow context for a specific rethrow.
     *
     * If \p rethrowNumber is out of range (negative or >= GetNumberOfRethrows()),
     * the output parameters are set to empty/zero values.
     *
     * \param[in]  rethrowNumber Zero-based index of the rethrow to query.
     * \param[out] file          Filled with the source file of the specified rethrow.
     * \param[out] line          Filled with the line number of the specified rethrow.
     * \param[out] message       Filled with the message of the specified rethrow.
     *
     * \sa AddRethrowData, GetNumberOfRethrows
     */
    void GetRethrowData(int rethrowNumber, std::string &file, int &line, std::string &message);

    /**
     * \brief Append data to the exception description via the stream operator.
     * \tparam T Type of the data to append (must be streamable to std::ostream).
     * \param[in] data The data to append to the description string.
     * \return Reference to this exception for chaining.
     */
    template <class T>
    inline Exception &operator<<(const T &data)
    {
      std::stringstream ss;
      ss << this->GetDescription() << data;
      this->SetDescription(ss.str());
      return *this;
    }

    /**
     * \brief Append non-const data to the exception description via the stream operator.
     * \tparam T Type of the data to append (must be streamable to std::ostream).
     * \param[in] data The data to append to the description string.
     * \return Reference to this exception for chaining.
     */
    template <class T>
    inline Exception &operator<<(T &data)
    {
      std::stringstream ss;
      ss << this->GetDescription() << data;
      this->SetDescription(ss.str());
      return *this;
    }

    /**
     * \brief Append a stream manipulator (e.g. std::endl) to the exception description.
     * \param[in] func A stream manipulator function pointer.
     * \return Reference to this exception for chaining.
     */
    inline Exception &operator<<(std::ostream &(*func)(std::ostream &))
    {
      std::stringstream ss;
      ss << this->GetDescription() << func;
      this->SetDescription(ss.str());
      return *this;
    }

  protected:
    /** \brief Internal storage for a single rethrow record. */
    struct ReThrowData
    {
      std::string RethrowClassname;  ///< Source file of the rethrow.
      unsigned int RethrowLine;      ///< Line number of the rethrow.
      std::string RethrowMessage;    ///< Message associated with the rethrow.
    };

    /** \brief List of recorded rethrow contexts. */
    std::vector<ReThrowData> m_RethrowData;
  };

  /**
   * \brief Stream output operator for mitk::Exception.
   *
   * Writes the exception description to the given output stream.
   *
   * \param[in,out] os The output stream.
   * \param[in]     e  The exception to write.
   * \return Reference to the output stream.
   */
  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &os, const mitk::Exception &e);
} // namespace mitk

#endif
