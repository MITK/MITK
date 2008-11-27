SET(CPP_BASE_FILENAMES
 AbstractConfiguration
 Application
 ConfigurationMapper
 ConfigurationView
 HelpFormatter
 IniFileConfiguration 
 LayeredConfiguration
 LoggingConfigurator
 LoggingSubsystem
 MapConfiguration
 Option
 OptionException
 OptionProcessor
 OptionSet
 PropertyFileConfiguration
 Subsystem
 SystemConfiguration
 XMLConfiguration
 FilesystemConfiguration
 ServerApplication
 Validator
 IntValidator
 RegExpValidator
 OptionCallback
)

IF(WIN32)
  SET(CPP_BASE_FILENAMES ${CPP_BASE_FILENAMES} WinService)
ENDIF(WIN32)

FOREACH(basename ${CPP_BASE_FILENAMES})
  SET(CPP_FILES ${CPP_FILES} src/${basename})
ENDFOREACH(basename ${CPP_BASE_FILENAMES})