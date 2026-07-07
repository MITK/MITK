/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLog_h
#define mitkLog_h

#include <mitkLogBackendBase.h>

#include <sstream>

#include <MitkLogExports.h>

#ifndef MITKLOG_MODULENAME
#  if defined(US_MODULE_NAME)
#    define MITKLOG_STR_(x) #x
#    define MITKLOG_STR(x) MITKLOG_STR_(x)
#    define MITKLOG_MODULENAME MITKLOG_STR(US_MODULE_NAME)
#  else
#    define MITKLOG_MODULENAME "n/a"
#  endif
#endif

namespace mitk
{
  /** \brief Register a log backend in the MITK log mechanism.
   *
   * Once registered, the backend receives all log messages through its ProcessMessage()
   * method (provided its OutputType is enabled). Multiple backends can be registered
   * simultaneously.
   *
   * If no backend is registered when a message is emitted, a default LogBackendCout is
   * created automatically. This default backend is removed when a second backend is added.
   *
   * \param[in] backend Pointer to the backend to register. The caller retains ownership
   *                    and must ensure the backend remains valid until UnregisterBackend()
   *                    is called.
   *
   * \pre \p backend is not null.
   *
   * \sa UnregisterBackend(), LogBackendBase
   */
  void MITKLOG_EXPORT RegisterBackend(LogBackendBase* backend);

  /** \brief Unregister a previously registered log backend.
   *
   * After unregistration, the backend will no longer receive log messages.
   * If the backend was not previously registered, this is a no-op.
   *
   * \param[in] backend Pointer to the backend to unregister.
   *
   * \sa RegisterBackend()
   */
  void MITKLOG_EXPORT UnregisterBackend(LogBackendBase* backend);

  /** \brief Distribute a log message to all registered and enabled backends.
   *
   * Trims trailing whitespace from the message text, then iterates over all
   * registered backends and calls ProcessMessage() on each whose OutputType is enabled.
   *
   * If no backend is registered, a default LogBackendCout is created and registered
   * automatically to ensure output is never silently lost.
   *
   * \note This function should only be called internally by PseudoLogStream upon destruction.
   *       Do not call directly; use the MITK_INFO, MITK_WARN, MITK_ERROR, MITK_FATAL, or
   *       MITK_DEBUG macros instead.
   *
   * \param[in,out] message The log message to distribute. The Message field may be modified
   *                        (trailing whitespace is trimmed).
   */
  void MITKLOG_EXPORT DistributeToBackends(LogMessage& message);

  /** \brief Enable all backends of the given output type.
   *
   * Backends of this type will resume receiving log messages. All output types are
   * enabled by default.
   *
   * \param[in] type The output type to enable.
   *
   * \sa DisableBackends(), IsBackendEnabled()
   */
  void MITKLOG_EXPORT EnableBackends(LogBackendBase::OutputType type);

  /** \brief Disable all backends of the given output type.
   *
   * Backends of this type will stop receiving log messages until re-enabled.
   * The backends remain registered but are skipped during message distribution.
   *
   * \param[in] type The output type to disable.
   *
   * \sa EnableBackends(), IsBackendEnabled()
   */
  void MITKLOG_EXPORT DisableBackends(LogBackendBase::OutputType type);

  /** \brief Check whether backends of the given output type are enabled.
   *
   * \param[in] type The output type to query.
   *
   * \return \c true if backends of this type are enabled (i.e. will receive messages),
   *         \c false if they have been disabled via DisableBackends().
   *
   * \sa EnableBackends(), DisableBackends()
   */
  bool MITKLOG_EXPORT IsBackendEnabled(LogBackendBase::OutputType type);

  /** \brief Stream-based log message builder for the MITK log mechanism.
   *
   * Provides an \c std::ostream-like interface for constructing log messages.
   * Data streamed via operator<< is accumulated in an internal string stream.
   * Upon destruction, the assembled message is distributed to all registered
   * and enabled backends via DistributeToBackends().
   *
   * All streaming operations use the "C" locale to ensure consistent numeric
   * formatting regardless of the application's locale settings.
   *
   * This class should not be instantiated directly. Use the convenience macros
   * MITK_INFO, MITK_WARN, MITK_ERROR, MITK_FATAL, and MITK_DEBUG instead.
   *
   * Example:
   * \code
   * MITK_INFO << "Processing image " << imageIndex << " of " << totalImages;
   * MITK_WARN("myCategory") << "Unexpected value: " << value;
   * MITK_DEBUG(verbose) << "Detailed debug output";
   * \endcode
   *
   * \sa NullLogStream, LogMessage, DistributeToBackends()
   */
  class MITKLOG_EXPORT PseudoLogStream
  {
  public:
    /** \brief Construct a log stream for a message at the given level and source location.
     *
     * \param[in] level        Severity level of the log message.
     * \param[in] filePath     Source file path (typically \c __FILE__).
     * \param[in] lineNumber   Source line number (typically \c __LINE__).
     * \param[in] functionName Source function name (typically \c __FUNCTION__).
     */
    PseudoLogStream(LogLevel level, const std::string& filePath, int lineNumber, const std::string& functionName)
      : m_Disabled(false),
        m_Message(level, filePath, lineNumber, functionName),
        m_Stream(std::stringstream::out)
    {
    }

    /** \brief Destructor that distributes the assembled log message to all backends.
     *
     * If the stream has not been disabled, the accumulated message text and the
     * module name (from \c MITKLOG_MODULENAME) are set on the internal LogMessage,
     * which is then passed to DistributeToBackends().
     */
    ~PseudoLogStream()
    {
      if (!m_Disabled)
      {
        m_Message.Message = m_Stream.str();
        m_Message.ModuleName = MITKLOG_MODULENAME;
        DistributeToBackends(m_Message);
      }
    }

    /** \brief Stream data into the log message (const reference overload).
     *
     * Appends the string representation of \p data to the internal message stream
     * using the "C" locale. If the stream is disabled, this is a no-op.
     *
     * \tparam T    Type of the data to stream. Must support \c operator<< with \c std::ostream.
     * \param[in] data The data to append to the log message.
     * \return Reference to this stream for chaining.
     */
    template <class T>
    PseudoLogStream& operator<<(const T& data)
    {
      if (!m_Disabled)
      {
        std::locale C("C");
        std::locale originalLocale = m_Stream.getloc();
        m_Stream.imbue(C);

        m_Stream << data;

        m_Stream.imbue(originalLocale);
      }

      return *this;
    }

    /** \brief Stream data into the log message (non-const reference overload).
     *
     * Appends the string representation of \p data to the internal message stream
     * using the "C" locale. If the stream is disabled, this is a no-op.
     *
     * \tparam T    Type of the data to stream. Must support \c operator<< with \c std::ostream.
     * \param[in] data The data to append to the log message.
     * \return Reference to this stream for chaining.
     */
    template <class T>
    PseudoLogStream& operator<<(T& data)
    {
      if (!m_Disabled)
      {
        std::locale C("C");
        std::locale originalLocale = m_Stream.getloc();
        m_Stream.imbue(C);

        m_Stream << data;

        m_Stream.imbue(originalLocale);
      }

      return *this;
    }

    /** \brief Stream manipulator overload (e.g. \c std::endl, \c std::flush).
     *
     * Applies the given stream manipulator to the internal message stream.
     * If the stream is disabled, this is a no-op.
     *
     * \param[in] func A stream manipulator function (e.g. \c std::endl).
     * \return Reference to this stream for chaining.
     */
    PseudoLogStream& operator<<(std::ostream& (*func)(std::ostream&))
    {
      if (!m_Disabled)
      {
        std::locale C("C");
        std::locale originalLocale = m_Stream.getloc();
        m_Stream.imbue(C);

        m_Stream << func;

        m_Stream.imbue(originalLocale);
      }

      return *this;
    }

    /** \brief Set or append a category for this log message.
     *
     * Categories help organize log output by topic or subsystem.
     * If a category has already been set, the new category is appended
     * with a dot separator (e.g. calling with "a" then "b" yields "a.b").
     *
     * If the stream is disabled, this is a no-op.
     *
     * \param[in] category The category string to set or append.
     * \return Reference to this stream for chaining.
     *
     * Example:
     * \code
     * MITK_INFO("io")("dicom") << "Loading DICOM file";
     * // Category will be "io.dicom"
     * \endcode
     */
    PseudoLogStream& operator()(const std::string& category)
    {
      if (!m_Disabled)
      {
        if (m_Message.Category.length())
          m_Message.Category += ".";

        m_Message.Category += category;
      }

      return *this;
    }

    /** \brief Conditionally enable or disable this log stream.
     *
     * When called with \c false, the stream is disabled: all subsequent streaming
     * operations become no-ops and the message will not be distributed to backends.
     * Once disabled, the stream cannot be re-enabled.
     *
     * \param[in] enabled If \c false, disables this stream. If \c true, no effect
     *                    (the stream remains in its current state).
     * \return Reference to this stream for chaining.
     *
     * Example:
     * \code
     * MITK_INFO(verbose) << "This is only logged when verbose is true";
     * \endcode
     */
    PseudoLogStream& operator()(bool enabled)
    {
      m_Disabled |= !enabled;
      return *this;
    }

  protected:
    bool m_Disabled;
    LogMessage m_Message;
    std::stringstream m_Stream;
  };

  /** \brief No-op log stream that silently discards all input.
   *
   * Used as a replacement for PseudoLogStream when debug logging is disabled
   * (i.e. when \c MITK_ENABLE_DEBUG_MESSAGES is not defined). All streaming
   * operations and category/enable calls are no-ops that compile to nothing,
   * ensuring zero runtime overhead for disabled debug messages.
   *
   * This class should not be used directly. It is used internally by the
   * MITK_DEBUG macro.
   *
   * \sa PseudoLogStream
   */
  class MITKLOG_EXPORT NullLogStream
  {
  public:
    /** \brief No-op stream operator (const reference overload). */
    template <class T>
    NullLogStream& operator<<(const T&)
    {
      return *this;
    }

    /** \brief No-op stream operator (non-const reference overload). */
    template <class T>
    NullLogStream& operator<<(T&)
    {
      return *this;
    }

    /** \brief No-op stream manipulator operator. */
    NullLogStream& operator<<(std::ostream &(*)(std::ostream &))
    {
      return *this;
    }

    /** \brief No-op category operator. */
    NullLogStream& operator()(const char*)
    {
      return *this;
    }

    /** \brief No-op enable/disable operator. */
    NullLogStream& operator()(bool)
    {
      return *this;
    }
  };
}

/** \brief Log an informational message.
 *
 * Creates a PseudoLogStream at LogLevel::Info with the current source location.
 * Usage: \code MITK_INFO << "message"; \endcode
 * Supports categories: \code MITK_INFO("category") << "message"; \endcode
 * Supports conditional output: \code MITK_INFO(condition) << "message"; \endcode
 */
#define MITK_INFO mitk::PseudoLogStream(mitk::LogLevel::Info, __FILE__, __LINE__, __FUNCTION__)

/** \brief Log a warning message.
 *
 * Creates a PseudoLogStream at LogLevel::Warn with the current source location.
 * Usage: \code MITK_WARN << "message"; \endcode
 */
#define MITK_WARN mitk::PseudoLogStream(mitk::LogLevel::Warn, __FILE__, __LINE__, __FUNCTION__)

/** \brief Log an error message.
 *
 * Creates a PseudoLogStream at LogLevel::Error with the current source location.
 * Usage: \code MITK_ERROR << "message"; \endcode
 */
#define MITK_ERROR mitk::PseudoLogStream(mitk::LogLevel::Error, __FILE__, __LINE__, __FUNCTION__)

/** \brief Log a fatal error message.
 *
 * Creates a PseudoLogStream at LogLevel::Fatal with the current source location.
 * Usage: \code MITK_FATAL << "message"; \endcode
 */
#define MITK_FATAL mitk::PseudoLogStream(mitk::LogLevel::Fatal, __FILE__, __LINE__, __FUNCTION__)

#ifdef MITK_ENABLE_DEBUG_MESSAGES
/** \brief Log a debug message (only active when MITK_ENABLE_DEBUG_MESSAGES is defined).
 *
 * When debug messages are enabled, creates a PseudoLogStream at LogLevel::Debug.
 * When disabled, evaluates to a NullLogStream that discards all input at zero cost.
 * Usage: \code MITK_DEBUG << "message"; \endcode
 */
#define MITK_DEBUG mitk::PseudoLogStream(mitk::LogLevel::Debug, __FILE__, __LINE__, __FUNCTION__)
#else
#define MITK_DEBUG true ? mitk::NullLogStream() : mitk::NullLogStream()
#endif

#endif
