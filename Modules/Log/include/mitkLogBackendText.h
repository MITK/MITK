/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogBackendText_h
#define mitkLogBackendText_h

#include <mitkLogBackendBase.h>

namespace mitk
{
  /** \brief Abstract superclass for text-based log backends.
   *
   * Provides two formatting modes for log messages that subclasses can use:
   *
   * - **Smart/short format**: Compact output with a relative timestamp (seconds since
   *   process start), optional category, log level prefix (for non-Info levels), and
   *   the message text. On Windows, this format uses colored console output.
   *   Level indicators: \c [] for Info, \c !! for Warn, \c ## for Error,
   *   \c ** for Fatal, \c {} for Debug.
   *
   * - **Full/long format**: Verbose pipe-delimited output containing the log level name,
   *   absolute timestamp, file path, line number, function name, thread ID, module name,
   *   category, and message text. Suitable for machine-readable log analysis.
   *
   * Subclasses must implement ProcessMessage() and typically delegate to one of the
   * Format methods.
   *
   * \sa LogBackendCout, LogBackendBase
   */
  class MITKLOG_EXPORT LogBackendText : public LogBackendBase
  {
  public:
    ~LogBackendText() override;

    void ProcessMessage(const LogMessage& message) override = 0;

  protected:
    /** \brief Format the given message in smart/short format and write it to std::cout.
     *
     * On Windows, this delegates to FormatSmartWindows() for colored console output.
     * On other platforms, it delegates to FormatSmart(std::ostream&, ...) with std::cout.
     *
     * On the very first call, the current wall-clock time is printed as a reference timestamp.
     *
     * \param[in] message  The log message to format and output.
     * \param[in] threadID Optional thread ID where the message was emitted. Defaults to 0
     *                     (unused in smart format output).
     */
    void FormatSmart(const LogMessage& message, int threadID = 0);

    /** \brief Format the given message in full/long format and write it to std::cout.
     *
     * Delegates to FormatFull(std::ostream&, ...) with std::cout.
     *
     * \param[in] message  The log message to format and output.
     * \param[in] threadID Optional thread ID where the message was emitted. Defaults to 0.
     *                     Written as a hexadecimal value in the output.
     */
    void FormatFull(const LogMessage& message, int threadID = 0);

    /** \brief Format the given message in smart/short format and write it to the given stream.
     *
     * Output format: \c {open_char}{relative_time}{close_char} [category] LEVEL: message
     *
     * The opening and closing characters vary by log level (e.g. \c [] for Info, \c !! for Warn).
     * On the very first invocation, the current wall-clock time is written as a reference line.
     *
     * \param[out] out      The output stream to write the formatted message to.
     * \param[in]  message  The log message to format.
     * \param[in]  threadID Optional thread ID (currently unused in the smart format). Defaults to 0.
     */
    void FormatSmart(std::ostream& out, const LogMessage& message, int threadID = 0);

    /** \brief Format the given message in full/long format and write it to the given stream.
     *
     * Output format (pipe-delimited):
     * \c LEVEL|timestamp||filePath(lineNumber)|functionName|threadID_hex|moduleName|category message
     *
     * \param[out] out      The output stream to write the formatted message to.
     * \param[in]  message  The log message to format.
     * \param[in]  threadID Optional thread ID. Written as hexadecimal. Defaults to 0.
     */
    void FormatFull(std::ostream& out, const LogMessage& message, int threadID = 0);

    /** \brief Append the current wall-clock time to the given stream.
     *
     * Writes the result of \c ctime() (with trailing newline replaced by a space)
     * to the stream.
     *
     * \param[out] out The output stream to append the timestamp to.
     */
    void AppendTimeStamp(std::ostream& out);

    /** \brief Format a message in smart/short format with colored Windows console output.
     *
     * Uses Windows Console API to colorize different parts of the log output
     * (timestamp, category, level, message) based on the log level. This method
     * is only available on Windows (compiled when \c MITK_WIN32_CONSOLE_COLOR is defined).
     *
     * \param[in] message  The log message to format and output.
     * \param[in] threadID Thread ID (currently unused).
     */
    void FormatSmartWindows(const LogMessage& message, int threadID);
  };
}

#endif
