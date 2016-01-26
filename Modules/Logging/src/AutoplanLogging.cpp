#include "AutoplanLogging.h"

namespace Logger {

  Options::Options()
  {
    // defaults
    consolelog = true;
    filelog = true;
    tcplog = false;
    datastoragelog = true;

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

  std::string Log::getData() const
  {
    dataBackend->flush();
    return dataStream->str();
  }
}
