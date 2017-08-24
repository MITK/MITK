MITK_CREATE_MODULE( IGTUIConfigurationWidgetRegistry
  INCLUDE_DIRS
    PRIVATE src/ConfigurationWidgetRegistry
  DEPENDS PRIVATE MitkIGTUI
  AUTOLOAD_WITH MitkIGTUI
)