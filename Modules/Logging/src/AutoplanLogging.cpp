#include "AutoplanLogging.h"

#include <iostream>
#include <cstdlib>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/date_time.hpp>

namespace Logger {

  Options::Options()
  {
    // defaults
    consolelog = true;
    filelog = true;
    tcplog = false;
    datastoragelog = false;

#ifdef _WIN32
    if(const char* ifAppData = std::getenv("PATH")) {
      logsPath = std::string(ifAppData) + "\\DKFZ\\logs\\";
    }
    else {
      logsPath = "./logs/";
    }
#else
    if(const char* ifHome = std::getenv("PATH")) {
      logsPath = std::string(ifHome) + "/.local/share/DKFZ/logs/";
    }
    else {
      logsPath = "./logs/";
    }
#endif
  }

  Options& Options::get()
  {
    static Options INSTANCE;
    return INSTANCE;
  }

  std::string Options::getIphost() const
  {
    return iphost;
  }

  std::string Options::getIpport() const
  {
    return ipport;
  }

  std::string Options::getLogsPath() const
  {
    return logsPath;
  }

  Log::Log()
  {
    dataBackend =
      boost::make_shared< boost::log::sinks::text_ostream_backend >();

    dataStream =
      boost::make_shared< std::stringstream >();

    reinitLogger();
  }

  boost::shared_ptr< std::stringstream > Log::getDataStream() const
  {
    return dataStream;
  }

  boost::shared_ptr< boost::log::sinks::text_ostream_backend > Log::getDataBackend() const
  {
    return dataBackend;
  }

  Log& Log::get()
  {
    static Log INSTANCE;
    return INSTANCE;
  }

  void Log::reinitLogger() const
  {
    boost::log::core::get()->flush();
    boost::log::core::get()->remove_all_sinks();

    /// Just return if everything is disabled
    if (!(Options::get().consolelog || Options::get().filelog || Options::get().tcplog || Options::get().datastoragelog)) return;

    if (Options::get().filelog) {
      boost::shared_ptr< file_sink > sink(new file_sink(
        boost::log::keywords::file_name = Options::get().logsPath + "/%Y%m%d_%H%M%S_%5N.xml",
        boost::log::keywords::rotation_size = 16384
        ));
      sink->locked_backend()->set_file_collector(boost::log::sinks::file::make_collector(
        boost::log::keywords::target = Options::get().logsPath,     /*< the target directory >*/
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
      sink3->set_formatter(
        boost::log::expressions::format("%1% > %2%")
        % boost::log::expressions::attr< boost::posix_time::ptime >("TimeStamp")
        % boost::log::expressions::xml_decor[boost::log::expressions::stream << boost::log::expressions::smessage]
        );

      boost::log::core::get()->add_sink(sink3);
    }

    if (Options::get().consolelog) {
      boost::log::add_console_log(std::cout,
        boost::log::keywords::format = "[%TimeStamp%] %Message%");
    }

    boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
    boost::log::core::get()->add_global_attribute("RecordID", boost::log::attributes::counter< unsigned int >());

    boost::log::add_common_attributes();
  }

  // reutrns true in case of date time parse success
  std::tuple<bool, boost::posix_time::ptime> dateFromString(const std::string& dateTime)
  {
    std::string formatDate("%Y-%b-%d %H:%M:%S%F"); // 2016-Jan-27 18:04:30.610194
    boost::posix_time::ptime date;
    try {
      std::istringstream stream(dateTime);
      stream.imbue(std::locale(std::locale::classic(),
        new boost::gregorian::date_input_facet(formatDate.c_str())));
      stream >> date;
    }
    catch (const std::exception& excp) {
      std::string text = excp.what();
      std::cout << text << std::endl;
      std::make_tuple(false, date);
    }
    return std::make_tuple(true, date);
  }

  // get actual seconds from boost::posix_time
  std::time_t pt_to_time_t(const boost::posix_time::ptime& pt)
  {
    boost::posix_time::ptime timet_start(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::time_duration diff = pt - timet_start;
    return diff.ticks() / boost::posix_time::time_duration::rep_type::ticks_per_second;
  }

  std::string Log::getLastDateTime(std::string str)
  {
    std::vector<std::string> lines;
    boost::split(lines, str, boost::is_any_of("\n"));
    if (lines.size() > 1) {
      std::string tail = lines.end()[-2];
      std::vector<std::string> message;
      boost::split(message, tail, boost::is_any_of(" >"));
      if (message.size() > 1) {
        std::string date = message.front();
        std::string time = message.begin()[1];
        return (date + " " + time);
      }
    }
    return nullptr;
  }

  void Log::resetData() const
  {
    dataBackend->remove_stream(dataStream);
    dataStream->str("");
    dataStream->clear();
    dataBackend->add_stream(dataStream);
  }

  std::string Log::getData() const
  {
    dataBackend->flush();
    return dataStream->str();
  }

  std::string Log::getDataFromDate(std::string dateTime) const
  {
    dataBackend->flush();
    std::vector<std::string> lines;
    std::string datastring = dataStream->str();
    boost::split(lines, datastring, boost::is_any_of("\n"));
    if (lines.size() > 1) {
      std::vector<std::string> result;

      std::vector<std::string>::reverse_iterator rit = lines.rbegin();
      for (; rit != lines.rend(); ++rit) {
        std::vector<std::string> message;
        boost::split(message, *rit, boost::is_any_of(" >"));
        if (message.size() > 1) {
          std::string date = message.front();
          std::string time = message.begin()[1];
          std::string lineTime = (date + " " + time);

          auto lastDate = dateFromString(dateTime);
          auto lineDate = dateFromString(lineTime);

          if ( std::get<0>(lastDate) == true
            && std::get<0>(lineDate) == true) {
            std::time_t lastDateTime = pt_to_time_t( std::get<1>(lastDate) );
            std::time_t lastLineTime = pt_to_time_t( std::get<1>(lineDate) );
            if (lastDateTime < lastLineTime) {
              result.push_back(*rit);
            }
            else {
              break;
            }
          }
        }
      }

      std::reverse(result.begin(), result.end());
      return boost::join(result, "\n") + "\n";
    }
    return nullptr;
  }
}
