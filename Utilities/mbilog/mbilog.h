/*=========================================================================

Program:   mbilog - logging for mitk / openCherry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _MBILOG_H
#define _MBILOG_H

#include <string>
#include <iostream>
#include <sstream>

#ifndef MBILOG_MODULENAME
  #if defined(MITK_MODULENAME)
    #define MBILOG_MODULENAME MITK_MODULENAME
  #else
    #define MBILOG_MODULENAME "n/a"
  #endif
#endif

#if defined(_WIN32)
  #ifdef mbilog_EXPORTS
    #define MBILOG_DLL_API __declspec(dllexport)
  #else
    #define MBILOG_DLL_API __declspec(dllimport)
  #endif
#else
  #define MBILOG_DLL_API
#endif

namespace mbilog {

  enum {
    Info,
    Warn,
    Error,
    Fatal,
    Debug
  };

  class MBILOG_DLL_API LogMessage {

    public:

      const int level;
      const char* filePath;
      const int lineNumber;
      const char* functionName;
      
      const char* moduleName;
      std::string category;
      std::string message;

      LogMessage(
        const int _level,
        const char* _filePath,
        const int _lineNumber,
        const char* _functionName
      )
        : level(_level)
        , filePath(_filePath)
        , lineNumber(_lineNumber)
        , functionName(_functionName)
      {
      }
  };

  struct MBILOG_DLL_API AbstractBackend
  {
    virtual ~AbstractBackend(){}
    virtual void ProcessMessage(const mbilog::LogMessage& )=0;
  };

  class MBILOG_DLL_API BackendCout : public AbstractBackend
  {
    public:

      BackendCout();
      ~BackendCout();
      virtual void ProcessMessage(const mbilog::LogMessage &l );

      void SetFull(bool full);

      static void FormatSmart(const LogMessage &l,int threadID=0);
      static void FormatFull(const LogMessage &l,int threadID=0);

    private:

      bool useFullOutput;

  };

  void MBILOG_DLL_API RegisterBackend(AbstractBackend* backend);
  void MBILOG_DLL_API UnregisterBackend(AbstractBackend* backend);
  void MBILOG_DLL_API DistributeToBackends(const LogMessage &l);

  class MBILOG_DLL_API PseudoStream {

    protected:

      bool disabled;
      LogMessage msg;
      std::stringstream ss;

    public:

      inline PseudoStream( int level,
                    const char* filePath,
                    int lineNumber,
                    const char* functionName)
                          : disabled(false)
                          , msg(LogMessage(level,filePath,lineNumber,functionName))
                          , ss(std::stringstream::out)
      {
      }

      inline ~PseudoStream()
      {
        if(!disabled)
        {
          msg.message = ss.str();
          msg.moduleName = MBILOG_MODULENAME;
          DistributeToBackends(msg);
        }
      }

      template <class T> inline PseudoStream& operator<<(const T& data) 
      {
        if(!disabled)
          ss << data;
        return *this;
      }
      
      template <class T> inline PseudoStream& operator<<(T& data)
      {
        if(!disabled)
          ss << data;
        return *this;
      }

      inline PseudoStream& operator<<(std::ostream& (*func)(std::ostream&))
      {
        if(!disabled)
          ss << func;
        return *this;
      }

      inline PseudoStream& operator()(const char *category)
      {
        if(!disabled)
        {
          if(msg.category.length())
            msg.category+=".";
          msg.category+=category;
        }
        return *this;
      }

      inline PseudoStream& operator()(bool enabled)
      {
        disabled|=!enabled;
        return *this;
      }
  };

  class MBILOG_DLL_API NullStream {

    public:

      template <class T> inline NullStream& operator<<(const T& data)
      {
        return *this;
      }
      template <class T> inline NullStream& operator<<(T& data)
      {
        return *this;
      }
      inline NullStream& operator<<(std::ostream& (*)(std::ostream&))
      {
        return *this;
      }
      inline NullStream& operator()(const char *)
      {
        return *this;
      }
      inline NullStream& operator()(bool)
      {
        return *this;
      }
  };

//
//  template<typename T>
//  struct DelegateBackend : public AbstractBackend
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


#define LOG_INFO mbilog::PseudoStream(mbilog::Info,__FILE__,__LINE__,__FUNCTION__)
#define LOG_WARN mbilog::PseudoStream(mbilog::Warn,__FILE__,__LINE__,__FUNCTION__)
#define LOG_ERROR mbilog::PseudoStream(mbilog::Error,__FILE__,__LINE__,__FUNCTION__)
#define LOG_FATAL mbilog::PseudoStream(mbilog::Fatal,__FILE__,__LINE__,__FUNCTION__)

#ifdef MBILOG_ENABLE_DEBUG
#define LOG_DEBUG mbilog::PseudoStream(mbilog::Debug,__FILE__,__LINE__,__FUNCTION__)
#else
#define LOG_DEBUG true ? mbilog::NullStream() : mbilog::NullStream()
#endif


#endif
