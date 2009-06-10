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
    
      int level;
      const char* filePath;
      int lineNumber;
      const char* functionName;
      const char* moduleName;
      std::string category;
      std::string message;
  };

  struct MBILOG_DLL_API AbstractBackend
  {
    virtual ~AbstractBackend(){}
    virtual void ProcessMessage(const mbilog::LogMessage& )=0;
  };
  
  class MBILOG_DLL_API backendCout : public AbstractBackend
  {
    public:
    
      backendCout();
      ~backendCout();
      virtual void ProcessMessage(const mbilog::LogMessage &l );
      
      void setFull(bool full);

      static void formatSmart(const LogMessage &l,int threadID=0);
      static void formatFull(const LogMessage &l,int threadID=0);   
      
    private:
    
      bool useFullOutput;
      
  };
  
  void MBILOG_DLL_API RegisterBackend(AbstractBackend* backend);
  void MBILOG_DLL_API UnregisterBackend(AbstractBackend* backend);
  void MBILOG_DLL_API DistributeToBackends(const LogMessage &l);
  void MBILOG_DLL_API backend_std_cout_smart(const LogMessage &l);

  class MBILOG_DLL_API PseudoStream {

    protected:
    
      LogMessage msg;
      bool disabled;  
            
    public:

      inline PseudoStream( int level,
                    const char* filePath,
                    int lineNumber,
                    const char* functionName) 
                          : disabled(false)
      {
        msg.level = level;
        msg.filePath = filePath;
        msg.lineNumber = lineNumber;
        msg.functionName = functionName;
        msg.category = "";
        msg.message = "";
      }     

      inline ~PseudoStream()
      {
        if(!disabled)
        {
          msg.moduleName = MBILOG_MODULENAME;
          DistributeToBackends(msg);
        }  
      }

      template <class T> inline PseudoStream& operator<<(const T& data)
      {
        if(!disabled)
        {
          std::stringstream ss(std::stringstream::out);
          ss << data;
          msg.message += ss.str();
        }                 
        return *this;
      }
     
      inline PseudoStream& operator<<(std::ostream& (*func)(std::ostream&))
      {
        if(!disabled)
        {
          std::stringstream ss(std::stringstream::out);
          ss << func;
          msg.message += ss.str();
        }                 
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
      inline NullStream& operator<<(std::ostream& (*func)(std::ostream&))
      {
        return *this;
      }
      inline NullStream& operator()(const char *category)
      {
        return *this;
      }
      inline NullStream& operator()(bool enabled)
      {
        return *this;
      }
  };
  
                   /*
  template<typename T>
  struct DelegateBackend : public AbstractBackend
  {
    typedef void(T::*Func)(const mbilog::LogMessage&);
    
    T* m_obj;
    
    DelegateBackend(T* obj, T::Func func) : obj(obj), m_Func(func)
    { 
    }
    
    void ProcessMessage(const mbilog::LogMessage& msg)
    {
      obj->*m_Func(msg);
    }
  };  
                   */

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