#pragma once

#define BOOST_LOG_DYN_LINK 1

// boost
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/move/utility.hpp>
#include <boost/asio.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/locale/encoding.hpp>

// boost log
#include <boost/log/trivial.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>

// boost log::sources
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

// boost log::utility
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

// boost log::sinks
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

#include <MitkLoggingExports.h>

#define AUTOPLAN_INFO BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::info)
#define AUTOPLAN_ERROR BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::error)
#define AUTOPLAN_TRACE BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::trace)
#define AUTOPLAN_DEBUG BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::debug)
#define AUTOPLAN_WARNING BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::warning)
#define AUTOPLAN_FATAL BOOST_LOG_STREAM_SEV(Logger::Log::get().lg, boost::log::trivial::fatal)

namespace Logger
{
  typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend > file_sink;
  typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > ostream_sink;

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
      bool datastoragelog; // true for test

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

    public:
      boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg;
      boost::shared_ptr< std::stringstream > getDataStream() const;
      boost::shared_ptr< boost::log::sinks::text_ostream_backend > getDataBackend() const;

      static Log& get();

      void reinitLogger() const;

      static std::string getLastDateTime(std::string str);
      void resetData() const;
      std::string getData() const;
      std::string getDataFromDate(std::string dateTime) const;
  };
}
