#include "AutoplanLogging.h"

namespace Logger {
inline void reinitLogger()
{
  boost::log::core::get()->flush();
  boost::log::core::get()->remove_all_sinks();

#ifdef BERRY_LOG
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node("/ru.samsmu.log");
  if (prefs){
    iphost = prefs->Get("logstashIpAddress", "").toStdString();
    ipport = prefs->Get("logstashPort", "").toStdString();
    logsPath = prefs->Get("logsPath", "").toStdString();

    consolelog = prefs->GetBool("consoleLog", true);
    filelog = prefs->GetBool("fileLog", true);
    tcplog = prefs->GetBool("tcpLog", false);
  }
#endif

  /// Just return if everything is disabled
  if (!(consolelog || filelog || tcplog)) return;

#ifdef BERRY_LOG
  if (iphost.empty()) {
    iphost = "127.0.0.1";
    prefs->Put("logstashIpAddress", "127.0.0.1");
  }
  if (ipport.empty()) {
    ipport = "666";
    prefs->Put("logstashPort", "666");
  }
#endif

  boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg = my_logger::get();

  if (filelog) {
    if (logsPath.empty()) {
#ifdef _WIN32
      char* ifAppData = getenv("LOCALAPPDATA");
      if (ifAppData != nullptr) {
        logsPath = std::string(ifAppData) + "\\SamSMU\\logs\\";
        logsPath = boost::locale::conv::to_utf<char>(logsPath, "windows-1251");
      }
      else {
        logsPath = ".";
      }
#else
      char* ifHome = getenv("HOME");
      if (ifHome != nullptr) {
        logsPath = std::string(ifHome) + "/.local/share/SamSMU/logs/";
      }
      else {
        logsPath = ".";
      }
#endif

#ifdef BERRY_LOG
      prefs->Put("logsPath", logsPath.c_str());
#endif
    }
    boost::shared_ptr< file_sink > sink(new file_sink(
      boost::log::keywords::file_name = "%Y%m%d_%H%M%S_%5N.xml",
      boost::log::keywords::rotation_size = 16384
      ));
    sink->locked_backend()->set_file_collector(boost::log::sinks::file::make_collector(
      boost::log::keywords::target = logsPath,                    /*< the target directory >*/
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

  if (tcplog) {
    boost::shared_ptr< boost::log::sinks::text_ostream_backend > backend =
      boost::make_shared< boost::log::sinks::text_ostream_backend >();

    boost::shared_ptr< boost::asio::ip::tcp::iostream > stream =
      boost::make_shared< boost::asio::ip::tcp::iostream >();

    stream->connect(iphost, ipport);

    backend->add_stream(stream);
    backend->auto_flush(true);

    boost::shared_ptr< tcp_sink > sink2(new tcp_sink(backend));
    boost::log::core::get()->add_sink(sink2);
  }

  if (consolelog) {
    boost::log::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");
  }

  boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
  boost::log::core::get()->add_global_attribute("RecordID", boost::log::attributes::counter< unsigned int >());

  boost::log::add_common_attributes();
}
}