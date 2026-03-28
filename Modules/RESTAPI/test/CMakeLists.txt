include_directories(../src)
MITK_CREATE_MODULE_TESTS()

# Network tests bind to specific ports and can fail or stall under parallel execution.
set_property(TEST mitkRestServerTest PROPERTY RUN_SERIAL TRUE)
set_property(TEST mitkSecurityMiddlewareTest PROPERTY RUN_SERIAL TRUE)
