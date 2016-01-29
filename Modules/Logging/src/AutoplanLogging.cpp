#include "AutoplanLogging.h"

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

  };

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


  boost::log::sources::severity_logger<boost::log::trivial::severity_level> Log::getLog() const
  {
    return lg;
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
      AUTOPLAN_ERROR << text;
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

  void Log::resetData()
  {
    dataStream->clear();
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
