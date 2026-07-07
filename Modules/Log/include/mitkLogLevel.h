/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogLevel_h
#define mitkLogLevel_h

namespace mitk
{
  /** \brief Severity levels for log messages in the MITK log mechanism.
   *
   * Each level represents a different severity of log event. Backends may use these
   * levels to filter, format, or colorize output. The levels are ordered by severity
   * from least to most critical: Debug < Info < Warn < Error < Fatal.
   *
   * \sa LogMessage, PseudoLogStream
   */
  enum class LogLevel
  {
    Info,  /**< \brief Informational message for general progress and status updates. */
    Warn,  /**< \brief Warning about a potential problem that does not prevent operation. */
    Error, /**< \brief An error occurred but the application may continue. */
    Fatal, /**< \brief A critical error that typically requires the application to terminate. */
    Debug  /**< \brief Verbose diagnostic message, only emitted when MITK_ENABLE_DEBUG_MESSAGES is defined. */
  };
}

#endif
