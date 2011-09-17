SET(CPP_BASE_FILENAMES
 AbstractConfigurationTest ConfigurationTestSuite
 ConfigurationMapperTest ConfigurationViewTest Driver 
 HelpFormatterTest IniFileConfigurationTest LayeredConfigurationTest
 LoggingConfiguratorTest MapConfigurationTest
 OptionProcessorTest OptionSetTest OptionTest
 OptionsTestSuite PropertyFileConfigurationTest
 SystemConfigurationTest UtilTestSuite XMLConfigurationTest
 FilesystemConfigurationTest ValidatorTest
 TimerTestSuite TimerTest
)

IF(WIN32)
SET(CPP_BASE_FILENAMES
  WinDriver WindowsTestSuite WinConfigurationTest WinRegistryTest
  ${CPP_BASE_FILENAMES}
)
ENDIF(WIN32)


SET(CPP_FILES "")
FOREACH(basename ${CPP_BASE_FILENAMES})
  SET(CPP_FILES ${CPP_FILES} src/${basename})
ENDFOREACH(basename ${CPP_BASE_FILENAMES})
