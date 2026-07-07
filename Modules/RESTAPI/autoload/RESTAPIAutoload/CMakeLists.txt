mitk_create_module(RESTAPIAutoload
  DEPENDS MitkRESTAPI
  AUTOLOAD_WITH MitkQtWidgets #[[ Autoload with MitkQtWidgets, as we want to
                                  autoload the REST server in the MITK Workbench
                                  but not with headless command-line apps. ]]
)
