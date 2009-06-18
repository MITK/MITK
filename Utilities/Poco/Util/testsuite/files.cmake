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

SET(CPP_FILES "")
FOREACH(basename ${CPP_BASE_FILENAMES})
  SET(CPP_FILES ${CPP_FILES} src/${basename})
ENDFOREACH(basename ${CPP_BASE_FILENAMES})
