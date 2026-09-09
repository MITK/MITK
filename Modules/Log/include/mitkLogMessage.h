/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogMessage_h
#define mitkLogMessage_h

#include <mitkLogLevel.h>

#include <string>

#include <MitkLogExports.h>

namespace mitk
{
  /** \brief A single log message (log event) of the MITK log mechanism.
   *
   * Encapsulates all metadata and content for a single log event, including source
   * location, severity level, module name, optional category, and the message text.
   *
   * Log messages should only be created by the macros provided in mitkLog.h
   * (MITK_INFO, MITK_WARN, MITK_ERROR, MITK_FATAL, MITK_DEBUG) through the
   * PseudoLogStream class. Upon destruction of the stream, the message is distributed
   * to all registered backends via DistributeToBackends().
   *
   * \sa PseudoLogStream, LogBackendBase, DistributeToBackends()
   */
  struct MITKLOG_EXPORT LogMessage
  {
  public:
    /** \brief Construct a log message with source location information.
     *
     * \param[in] level    Severity level of the log message.
     * \param[in] filePath Absolute or relative path of the source file where the message was emitted.
     * \param[in] lineNumber Line number in the source file where the message was emitted.
     * \param[in] functionName Name of the function or method where the message was emitted.
     */
    LogMessage(const LogLevel level, const std::string& filePath, const int lineNumber, const std::string& functionName);

    /** \brief Severity level of the log message.
     *
     * Set at construction time and remains constant for the lifetime of the message.
     */
    const LogLevel Level;

    /** \brief Absolute or relative path of the source file where the log message was emitted.
     *
     * Typically populated by the \c __FILE__ preprocessor macro.
     */
    const std::string FilePath;

    /** \brief Line number in the source file where the log message was emitted.
     *
     * Typically populated by the \c __LINE__ preprocessor macro.
     */
    const int LineNumber;

    /** \brief Name of the function or method where the log message was emitted.
     *
     * Typically populated by the \c __FUNCTION__ preprocessor macro.
     */
    const std::string FunctionName;

    /** \brief Name of the MITK module where the log message was emitted.
     *
     * Automatically set by the PseudoLogStream constructor, from the
     * \c MITKLOG_MODULENAME macro as it resolves in the calling module.
     */
    std::string ModuleName;

    /** \brief Optional user-defined category for the log event.
     *
     * Categories can be set via the PseudoLogStream::operator()(const std::string&) call.
     * Multiple categories are concatenated with a dot separator (e.g. "module.subsystem").
     */
    std::string Category;

    /** \brief The actual log message text.
     *
     * Assembled from all data streamed into PseudoLogStream via operator<<.
     * Trailing whitespace is trimmed before distribution to backends.
     */
    std::string Message;
  };
}

#endif
