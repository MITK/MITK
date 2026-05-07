mitk_create_module(RESTAPIAutoload
  DEPENDS MitkRESTAPI
  AUTOLOAD_WITH MitkQtWidgets #bind it to MitkQtWidgets, as we want to have the rest autoload active with the workbench but not e.g. with every cli app
)
