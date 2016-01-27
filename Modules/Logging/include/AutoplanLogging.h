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

/// BERRY_LOG indicates that logger will write berry properties
#ifdef BERRY_LOG
#include <berryPlatform.h>
#endif

#define AUTOPLAN_INFO BOOST_LOG_STREAM_SEV(Logger::Log::get().getLog(), boost::log::trivial::info)
#define AUTOPLAN_ERROR BOOST_LOG_STREAM_SEV(Logger::Log::get().getLog(), boost::log::trivial::error)
#define AUTOPLAN_TRACE BOOST_LOG_STREAM_SEV(Logger::Log::get().getLog(), boost::log::trivial::trace)
#define AUTOPLAN_DEBUG BOOST_LOG_STREAM_SEV(Logger::Log::get().getLog(), boost::log::trivial::debug)
#define AUTOPLAN_WARNING BOOST_LOG_STREAM_SEV(Logger::Log::get().getLog(), boost::log::trivial::warning)
#define AUTOPLAN_FATAL BOOST_LOG_STREAM_SEV(Logger::Log::get().getLog(), boost::log::trivial::fatal)

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

      std::string iphost, ipport, logsPath;

    public:
      bool consolelog;
      bool filelog;
      bool tcplog;
      bool datastoragelog; // true for test

      static Options& get();

      std::string getIphost() const;
      std::string getIpport() const;
      std::string getLogsPath() const;

      friend class Log;
  };

  class MITKLOGGING_EXPORT Log
  {
    private:
      Log();
      Log(Log const&);
      void operator=(Log const&);

      boost::shared_ptr< std::stringstream > dataStream;
      boost::shared_ptr< boost::log::sinks::text_ostream_backend > dataBackend;

      boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg;
    public:
      boost::log::sources::severity_logger< boost::log::trivial::severity_level > getLog() const;
      boost::shared_ptr< std::stringstream > getDataStream() const;
      boost::shared_ptr< boost::log::sinks::text_ostream_backend > getDataBackend() const;

      static Log& get();

      void reinitLogger() const
      {
        boost::log::core::get()->flush();
        boost::log::core::get()->remove_all_sinks();

#ifdef BERRY_LOG
        berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
        berry::IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node("/ru.samsmu.log");
        if (prefs){
          Options::get().iphost = prefs->Get("logstashIpAddress", "").toStdString();
          Options::get().ipport = prefs->Get("logstashPort", "").toStdString();
          Options::get().logsPath = prefs->Get("logsPath", "").toStdString();

          Options::get().consolelog = prefs->GetBool("consoleLog", true);
          Options::get().filelog = prefs->GetBool("fileLog", true);
          Options::get().tcplog = prefs->GetBool("tcpLog", false);
        }
#endif

        /// Just return if everything is disabled
        if (!(Options::get().consolelog || Options::get().filelog || Options::get().tcplog || Options::get().datastoragelog)) return;

#ifdef BERRY_LOG
        if (Options::get().iphost.empty()) {
          Options::get().iphost = "127.0.0.1";
          prefs->Put("logstashIpAddress", "127.0.0.1");
        }
        if (Options::get().ipport.empty()) {
          Options::get().ipport = "666";
          prefs->Put("logstashPort", "666");
        }
#endif

        if (Options::get().filelog) {
          if (Options::get().logsPath.empty()) {
#ifdef _WIN32
            char* ifAppData = getenv("LOCALAPPDATA");
            if (ifAppData != nullptr) {
              Options::get().logsPath = std::string(ifAppData) + "\\SamSMU\\logs\\";
              Options::get().logsPath = boost::locale::conv::to_utf<char>(Options::get().logsPath, "windows-1251");
            }
            else {
              Options::get().logsPath = ".";
            }
#else
            char* ifHome = getenv("HOME");
            if (ifHome != nullptr) {
              Options::get().logsPath = std::string(ifHome) + "/.local/share/SamSMU/logs/";
            }
            else {
              Options::get().logsPath = ".";
            }
#endif

#ifdef BERRY_LOG
            prefs->Put("logsPath", Options::get().logsPath.c_str());
#endif
          }
          boost::shared_ptr< file_sink > sink(new file_sink(
            boost::log::keywords::file_name = "%Y%m%d_%H%M%S_%5N.xml",
            boost::log::keywords::rotation_size = 16384
            ));
          sink->locked_backend()->set_file_collector(boost::log::sinks::file::make_collector(
            boost::log::keywords::target = Options::get().logsPath,           /*< the target directory >*/
            boost::log::keywords::max_size = 16 * 1024 * 1024,          /*< maximum total size of the stored files, in bytes >*/
            boost::log::keywords::min_free_space = 100 * 1024 * 1024    /*< minimum free space on the drive, in bytes >*/
            ));
          sink->set_formatter(
            boost::log::expressions::format("\t<record id=\"%1%\" timestamp=\"%2%\">%3%</record>")
            % boost::log::expressions::attr< unsigned int >("RecordID")
            % boost::log::expressions::attr< boost::posix_time::ptime >("TimeStamp")
            % boost::log::expressions::xml_decor[boost::log::expressions::stream << boost::log::expressions::smessage]
            );

          auto write_header = [](boost::log::sinks::text_file_backend::stream_type& file) {
            file << "<?xml version=\"1.0\"?>\n<log>\n";
          };
          auto write_footer = [](boost::log::sinks::text_file_backend::stream_type& file) {
            file << "</log>\n";
          };
          /// Set header and footer writing functors
          sink->locked_backend()->set_open_handler(write_header);
          sink->locked_backend()->set_close_handler(write_footer);

          /// Add the sink to the core
          boost::log::core::get()->add_sink(sink);
        }

        if (Options::get().tcplog) {
          boost::shared_ptr< boost::log::sinks::text_ostream_backend > backend =
            boost::make_shared< boost::log::sinks::text_ostream_backend >();

          boost::shared_ptr< boost::asio::ip::tcp::iostream > stream =
            boost::make_shared< boost::asio::ip::tcp::iostream >();

          stream->connect(Options::get().iphost, Options::get().ipport);

          backend->add_stream(stream);
          backend->auto_flush(true);

          boost::shared_ptr< ostream_sink > sink2(new ostream_sink(backend));
          boost::log::core::get()->add_sink(sink2);
        }

        if (Options::get().datastoragelog) {
          dataBackend->add_stream(dataStream);

          boost::shared_ptr< ostream_sink > sink3(new ostream_sink(dataBackend));
          boost::log::core::get()->add_sink(sink3);
        }

        if (Options::get().consolelog) {
          boost::log::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");
        }

        boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
        boost::log::core::get()->add_global_attribute("RecordID", boost::log::attributes::counter< unsigned int >());

        boost::log::add_common_attributes();
      }

      std::string getData() const;
  };
}
