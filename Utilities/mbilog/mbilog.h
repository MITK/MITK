/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _MBILOG_H
#define _MBILOG_H

#include <sstream>

#include "mbilogBackendBase.h"
#include "mbilogBackendCout.h"
#include "mbilogConfig.h"
#include "mbilogExports.h"
#include "mbilogLogMessage.h"
#include "mbilogLoggingTypes.h"

namespace mbilog
{
  /** \brief Registeres a backend to the mbi logging mechanism. If a backend is registered here, all mbilog messages
   *         are relayed to this backend through the method ProcessMessage. If no backend is registered the default
   *         backend is used.
   */
  void MBILOG_EXPORT RegisterBackend(BackendBase *backend);

  /** \brief Unregisters a backend.
   */
  void MBILOG_EXPORT UnregisterBackend(BackendBase *backend);

  /** \brief Distributes the given message to all registered backends. Should only be called by objects
    *        of the class pseudo stream.
    */
  void MBILOG_EXPORT DistributeToBackends(LogMessage &l);

  /**
   * Enable the output of a backend.
   **/
  void MBILOG_EXPORT EnableBackends(OutputType type);
  /**
   * Disable the output of a backend.
   **/
  void MBILOG_EXPORT DisableBackends(OutputType type);
  /**
   * Checks wether the output of this backend is enabled.
   **/
  bool MBILOG_EXPORT IsBackendEnabled(OutputType type);

  /**
   * \brief An object of this class simulates a std::cout stream. This means messages can be added by
   *        using the bit shift operator (<<). Should only be used by the macros defined in the file mbilog.h
   *  \ingroup mbilog
   */
  class MBILOG_EXPORT PseudoStream
  {
  protected:
    bool disabled;
    LogMessage msg;
    std::stringstream ss;

  public:
    inline PseudoStream(int level, const char *filePath, int lineNumber, const char *functionName)
      : disabled(false), msg(LogMessage(level, filePath, lineNumber, functionName)), ss(std::stringstream::out)
    {
    }

    /** \brief The message which is stored in the member ss is written to the backend. */
    inline ~PseudoStream()
    {
      if (!disabled)
      {
        msg.message = ss.str();
        msg.moduleName = MBILOG_MODULENAME;
        DistributeToBackends(msg);
      }
    }

    /** \brief Definition of the bit shift operator for this class.*/
    template <class T>
    inline PseudoStream &operator<<(const T &data)
    {
      if (!disabled)
      {
        std::locale C("C");
        std::locale originalLocale = ss.getloc();
        ss.imbue(C);

        ss << data;

        ss.imbue(originalLocale);
      }
      return *this;
    }

    /** \brief Definition of the bit shift operator for this class (for non const data).*/
    template <class T>
    inline PseudoStream &operator<<(T &data)
    {
      if (!disabled)
      {
        std::locale C("C");
        std::locale originalLocale = ss.getloc();
        ss.imbue(C);

        ss << data;

        ss.imbue(originalLocale);
      }
      return *this;
    }

    /** \brief Definition of the bit shift operator for this class (for functions).*/
    inline PseudoStream &operator<<(std::ostream &(*func)(std::ostream &))
    {
      if (!disabled)
      {
        std::locale C("C");
        std::locale originalLocale = ss.getloc();
        ss.imbue(C);

        ss << func;

        ss.imbue(originalLocale);
      }
      return *this;
    }

    /** \brief Sets the category of this PseudoStream object. If there already is a category it is appended, seperated
     * by a dot.*/
    inline PseudoStream &operator()(const char *category)
    {
      if (!disabled)
      {
        if (msg.category.length())
          msg.category += ".";
        msg.category += category;
      }
      return *this;
    }

    /** \brief Enables/disables the PseudoStream. If set to false parsing and output is suppressed. */
    inline PseudoStream &operator()(bool enabled)
    {
      disabled |= !enabled;
      return *this;
    }
  };

  /**
   * \brief An object of this class simulates a std::cout stream but does nothing. This class is for dummy objects, bit
   * shift
   *        operators are availiable but doing nothing. Should only be used by the macros defined in the file mbilog.h
   * \ingroup mbilog
   */
  class MBILOG_EXPORT NullStream
  {
  public:
    template <class T>
    inline NullStream &operator<<(const T & /*data*/)
    {
      return *this;
    }
    template <class T>
    inline NullStream &operator<<(T & /*data*/)
    {
      return *this;
    }
    inline NullStream &operator<<(std::ostream &(*)(std::ostream &)) { return *this; }
    inline NullStream &operator()(const char *) { return *this; }
    inline NullStream &operator()(bool) { return *this; }
  };

  //  /** \brief templated backend: one can define a class and a method to create a new backend. */
  //  template<typename T>
  //  struct DelegateBackend : public BackendBase
  //  {
  //
  //    typedef void(T::*Callback)(const mbilog::LogMessage&);
  //
  //    DelegateBackend(T* obj, Callback callback) : m_Obj(obj), m_Callback(callback)
  //    {
  //    }
  //
  //    void ProcessMessage(const mbilog::LogMessage& msg)
  //    {
  //      m_Obj->*m_Callback(msg);
  //    }
  //
  //  private:
  //
  //    T* m_Obj;
  //    Callback m_Callback;
  //  };
}

/** \brief Macros for different message levels. Creates an instance of class PseudoStream with the corresponding message
 * level.
  *        Other parameters are the name of the source file, line of the source code and function name which are
 * generated
  *        by the compiler.
  */
#define MBI_INFO mbilog::PseudoStream(mbilog::Info, __FILE__, __LINE__, __FUNCTION__)
#define MBI_WARN mbilog::PseudoStream(mbilog::Warn, __FILE__, __LINE__, __FUNCTION__)
#define MBI_ERROR mbilog::PseudoStream(mbilog::Error, __FILE__, __LINE__, __FUNCTION__)
#define MBI_FATAL mbilog::PseudoStream(mbilog::Fatal, __FILE__, __LINE__, __FUNCTION__)

/** \brief Macro for the debug messages. The messages are disabled if the cmake variable MBILOG_ENABLE_DEBUG is false.
 */
#ifdef MBILOG_ENABLE_DEBUG
#define MBI_DEBUG mbilog::PseudoStream(mbilog::Debug, __FILE__, __LINE__, __FUNCTION__)
#else
#define MBI_DEBUG true ? mbilog::NullStream() : mbilog::NullStream() // this is magic by markus
#endif

#endif
