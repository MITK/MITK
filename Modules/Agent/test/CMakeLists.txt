MITK_CREATE_MODULE_TESTS(PACKAGE_DEPENDS PRIVATE httplib)

# Binds a fixed port.
set_property(TEST mitkMcpEndpointTest PROPERTY RUN_SERIAL TRUE)
