#pragma once

// #define BOOST_LOG_DYN_LINK

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

/// BERRY_LOG indicates that logger will write berry properties
#ifdef BERRY_LOG
#include <berryPlatform.h>
#endif

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, boost::log::sources::severity_logger< boost::log::trivial::severity_level >)

#define AUTOPLAN_INFO BOOST_LOG_STREAM_SEV(my_logger::get(), boost::log::trivial::info)
#define AUTOPLAN_ERROR BOOST_LOG_STREAM_SEV(my_logger::get(), boost::log::trivial::error)
#define AUTOPLAN_TRACE BOOST_LOG_STREAM_SEV(my_logger::get(), boost::log::trivial::trace)
#define AUTOPLAN_DEBUG BOOST_LOG_STREAM_SEV(my_logger::get(), boost::log::trivial::debug)
#define AUTOPLAN_WARNING BOOST_LOG_STREAM_SEV(my_logger::get(), boost::log::trivial::warning)
#define AUTOPLAN_FATAL BOOST_LOG_STREAM_SEV(my_logger::get(), boost::log::trivial::fatal)

namespace Logger {
  typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend > file_sink;
  typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > tcp_sink;

  static bool consolelog = true;
  static bool filelog = true;
  static bool tcplog = false;
  static std::string iphost, ipport, logsPath;

  extern void reinitLogger();
}
