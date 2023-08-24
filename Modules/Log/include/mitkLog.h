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
  /** \brief Register a backend in the MITK log mechanism.
   *
   * If a backend is registered here, all log messages are relayed to this backend through the method ProcessMessage.
   * If no backend is registered, the default backend is used.
   */
  void MITKLOG_EXPORT RegisterBackend(LogBackendBase* backend);

  /** \brief Unregister a backend.
   */
  void MITKLOG_EXPORT UnregisterBackend(LogBackendBase* backend);

  /** \brief Distribute the given message to all registered backends.
   *
   * Should only be called by PseudoLogStream objects.
   */
  void MITKLOG_EXPORT DistributeToBackends(LogMessage& message);

  /** \brief Enable the output of a backend.
   */
  void MITKLOG_EXPORT EnableBackends(LogBackendBase::OutputType type);

  /** \brief Disable the output of a backend.
   */
  void MITKLOG_EXPORT DisableBackends(LogBackendBase::OutputType type);

  /** \brief Check wether the output of this backend is enabled.
   */
  bool MITKLOG_EXPORT IsBackendEnabled(LogBackendBase::OutputType type);

  /** \brief Simulates a std::cout stream.
   *
   * Should only be used by the macros defined in the file mitkLog.h.
   */
  class MITKLOG_EXPORT PseudoLogStream
  {
  public:
    PseudoLogStream(LogLevel level, const std::string& filePath, int lineNumber, const std::string& functionName)
      : m_Disabled(false),
        m_Message(level, filePath, lineNumber, functionName),
        m_Stream(std::stringstream::out)
    {
    }

    /** \brief The encapsulated message is written to the backend.
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

    /** \brief Sets the category of this PseudoLogStream object.
     *
     * If there is already a category it is appended, seperated by a dot.
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

    /** \brief Enables/disables the PseudoLogStream.
     *
     * If set to false, parsing and output is suppressed.
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

  /**
   * \brief Simulates a std::cout stream but does nothing.
   *
   * Should only be used by the macros defined in the file mitkLog.h.
   */
  class MITKLOG_EXPORT NullLogStream
  {
  public:
    template <class T>
    NullLogStream& operator<<(const T&)
    {
      return *this;
    }

    template <class T>
    NullLogStream& operator<<(T&)
    {
      return *this;
    }

    NullLogStream& operator<<(std::ostream &(*)(std::ostream &))
    {
      return *this;
    }

    NullLogStream& operator()(const char*)
    {
      return *this;
    }

    NullLogStream& operator()(bool)
    {
      return *this;
    }
  };
}

#define MITK_INFO mitk::PseudoLogStream(mitk::LogLevel::Info, __FILE__, __LINE__, __FUNCTION__)
#define MITK_WARN mitk::PseudoLogStream(mitk::LogLevel::Warn, __FILE__, __LINE__, __FUNCTION__)
#define MITK_ERROR mitk::PseudoLogStream(mitk::LogLevel::Error, __FILE__, __LINE__, __FUNCTION__)
#define MITK_FATAL mitk::PseudoLogStream(mitk::LogLevel::Fatal, __FILE__, __LINE__, __FUNCTION__)

#ifdef MITK_ENABLE_DEBUG_MESSAGES
#define MITK_DEBUG mitk::PseudoLogStream(mitk::LogLevel::Debug, __FILE__, __LINE__, __FUNCTION__)
#else
#define MITK_DEBUG true ? mitk::NullLogStream() : mitk::NullLogStream()
#endif

#endif
