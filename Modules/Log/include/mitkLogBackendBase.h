/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogBackendBase_h
#define mitkLogBackendBase_h

#include <mitkLogMessage.h>
#include <MitkLogExports.h>

namespace mitk
{
  /** \brief Abstract interface for log backends that can be registered in the MITK log mechanism.
   *
   * Subclasses implement ProcessMessage() to handle log events (e.g. writing to console,
   * file, or a remote logging service). Backends are registered and unregistered via
   * mitk::RegisterBackend() and mitk::UnregisterBackend(). Backends of a given OutputType
   * can be collectively enabled or disabled via EnableBackends() / DisableBackends().
   *
   * If no backend is registered when a log message is emitted, a default LogBackendCout
   * instance is created automatically. This default backend is removed as soon as a
   * user-registered backend is added.
   *
   * \sa RegisterBackend(), UnregisterBackend(), LogBackendText, LogBackendCout
   */
  class MITKLOG_EXPORT LogBackendBase
  {
  public:
    /** \brief Classification of the output destination of a backend.
     *
     * Used by EnableBackends() / DisableBackends() to selectively enable or disable
     * groups of backends by their output type.
     */
    enum class OutputType
    {
      Console,    /**< \brief Backend writes to the console (stdout/stderr). */
      File,       /**< \brief Backend writes to a file. */
      Other = 100 /**< \brief Backend writes to another destination (e.g. network, GUI widget). */
    };

    virtual ~LogBackendBase();

    /** \brief Process a log message.
     *
     * Called by the MITK log mechanism for each registered and enabled backend
     * whenever a log message is emitted. Implementations should format and output
     * the message according to their destination.
     *
     * \param[in] message The log message to process. The message text has already
     *                    been trimmed of trailing whitespace.
     *
     * \pre The backend is registered via RegisterBackend() and its OutputType is enabled.
     */
    virtual void ProcessMessage(const LogMessage& message) = 0;

    /** \brief Return the output type of this backend.
     *
     * Used by the log mechanism to determine whether this backend is enabled.
     *
     * \return The OutputType classification of this backend.
     */
    virtual OutputType GetOutputType() const = 0;
  };
}

#endif
