set(CPP_BASE_FILENAMES
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

if(WIN32)
set(CPP_BASE_FILENAMES
  WinDriver WindowsTestSuite WinConfigurationTest WinRegistryTest
  ${CPP_BASE_FILENAMES}
)
endif(WIN32)


set(CPP_FILES "")
foreach(basename ${CPP_BASE_FILENAMES})
  set(CPP_FILES ${CPP_FILES} src/${basename})
endforeach(basename ${CPP_BASE_FILENAMES})
