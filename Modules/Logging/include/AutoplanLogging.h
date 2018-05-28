#pragma once

#include <string>
#include <algorithm>
#include <iterator>

#define BOOST_LOG_DYN_LINK 1

#define BOOST_USE_WINAPI_VERSION 0x0501

// boost
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/current_function.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/uuid/uuid.hpp>

// boost log
#include <boost/log/trivial.hpp>

// boost log::sources
#include <boost/log/sources/logger.hpp>

// boost log::sinks
#include <boost/log/sinks/text_ostream_backend.hpp>

// boost log::attributes
#include <boost/log/attributes/mutable_constant.hpp>

#include <ctime>

#include <MitkLoggingExports.h>

#include <ThreadPoolUtilities.h>

#include "StringUtilities.h"

// TODO add call BOOST_CURRENT_FUNCTION after investigate it behavior on Apple system
#ifdef __APPLE__
  #define AUTOPLAN_INFO BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::info)
  #define AUTOPLAN_ERROR BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::error)
  #define AUTOPLAN_TRACE BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::trace)
  #define AUTOPLAN_WARNING BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::warning)
  #define AUTOPLAN_FATAL BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::fatal)
#else
  #define AUTOPLAN_INFO BOOST_LOG_STREAM_SEV(Logger::Log::get(Logger::details::formatCallerName(BOOST_CURRENT_FUNCTION)).lg, boost::log::trivial::info)
  #define AUTOPLAN_ERROR BOOST_LOG_STREAM_SEV(Logger::Log::get(Logger::details::formatCallerName(BOOST_CURRENT_FUNCTION)).lg, boost::log::trivial::error)
  #define AUTOPLAN_TRACE BOOST_LOG_STREAM_SEV(Logger::Log::get(Logger::details::formatCallerName(BOOST_CURRENT_FUNCTION)).lg, boost::log::trivial::trace)
  #define AUTOPLAN_WARNING BOOST_LOG_STREAM_SEV(Logger::Log::get(Logger::details::formatCallerName(BOOST_CURRENT_FUNCTION)).lg, boost::log::trivial::warning)
  #define AUTOPLAN_FATAL BOOST_LOG_STREAM_SEV(Logger::Log::get(Logger::details::formatCallerName(BOOST_CURRENT_FUNCTION)).lg, boost::log::trivial::fatal)
#endif // __APPLE__

struct ThrowAwayPattern {};
MITKLOGGING_EXPORT extern struct ThrowAwayPattern _;

template <typename T>
ThrowAwayPattern & operator<<(ThrowAwayPattern&__, T)
{
  return __;
}

#ifdef DEBUG_INFO
  #ifdef __APPLE__
    #define AUTOPLAN_DEBUG BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::debug)
  #else
    #define AUTOPLAN_DEBUG BOOST_LOG_STREAM_SEV(Logger::Log::get(Logger::details::formatCallerName(BOOST_CURRENT_FUNCTION)).lg, boost::log::trivial::debug)
  #endif // __APPLE__
#else
  #define AUTOPLAN_DEBUG _
#endif

namespace Logger
{
  class MITKLOGGING_EXPORT Options
  {
    private:
      Options();
      Options(Options const&);
      void operator=(Options const&);

    public:
      std::string iphost, ipport, logsPath;

      bool consolelog;
      bool filelog;
      bool tcplog;
      bool datastoragelog;
      bool tcpdump;

      static Options& get();

      std::string getIphost() const;
      std::string getIpport() const;
      std::string getLogsPath() const;
  };

  class MITKLOGGING_EXPORT Log
  {
    private:
      Log();
      Log(Log const&);
      void operator=(Log const&);

      boost::shared_ptr< std::stringstream > dataStream;
      boost::shared_ptr< boost::log::sinks::text_ostream_backend > dataBackend;
      boost::log::attributes::mutable_constant<std::string> sourceAttribute;
      boost::log::attributes::mutable_constant<std::string> fullNameAttribute;
      boost::log::attributes::mutable_constant<std::string> organizationAttribute;
      boost::log::attributes::mutable_constant<boost::uuids::uuid> sessionTag;
      boost::log::attributes::mutable_constant<std::string> additionalField;

      Utilities::TaskGroup m_TaskGroup;

      clock_t m_StartTime;

    public:
      boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg;
      boost::shared_ptr< std::stringstream > getDataStream() const;
      boost::shared_ptr< boost::log::sinks::text_ostream_backend > getDataBackend() const;

      static Log& get();
      static Log& get(const std::string& src);

      void reinitLogger();

      void setSource(const std::string& src);
      void setUserData(const std::string& fullName, const std::string& organization);
      void setAdditionalField(const std::string& field, const std::string& value);
      void resetAdditionalField();

      static std::string getLastDateTime(std::string str);
      void resetData() const;
      std::string getData() const;
      std::string getDataFromDate(std::string dateTime) const;

      void setStartTime(clock_t time);
      void computeRunningTime(clock_t time);
  };

  namespace details
  {
    template<typename TTypeNameString>
    std::string formatCallerName(TTypeNameString&& typeName)
    {
      namespace xp = boost::xpressive;
      int level = 1;
      std::string formatedTypeName;
      //copy only function, classes and namespaces names
      std::copy_if(std::reverse_iterator<const char *>(typeName + strlen(typeName)), std::reverse_iterator<const char *>(typeName),
        std::back_inserter(formatedTypeName), [&level](const char& character) -> bool
      {
        if ('>' == character) {
          ++level;
          return false;
        }
        if ('<' == character) {
          --level;
          return false;
        }
        if ('(' == character) {
          --level;
          return false;
        }
        if (' ' == character) {
          if (!level)
            ++level;
          return false;
        }
        return !level;
      });
      //compare with case ClassName or ParentClassName::ClassName for ramove namespace name
      const xp::sregex classNameRegex = !(xp::as_xpr(':') >> *xp::_w >> xp::set[xp::range('A', 'Z')] >> ':') >> xp::as_xpr(':') >> *xp::_w >> xp::set[xp::range('A', 'Z')];
      xp::smatch searchResult;
      if (xp::regex_search(formatedTypeName, searchResult, classNameRegex)) {
        formatedTypeName.assign(searchResult[0].first + 1, searchResult[0].second);
      } else {
        formatedTypeName.resize(std::min(formatedTypeName.size(), formatedTypeName.find(':')));
      }
      std::reverse(formatedTypeName.begin(), formatedTypeName.end());
      return formatedTypeName;
    }
  }
}
