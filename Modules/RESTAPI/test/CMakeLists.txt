include_directories(../src)
MITK_CREATE_MODULE_TESTS()

# Network tests bind to specific ports and can fail or stall under parallel execution.
set_property(TEST mitkRestServerTest PROPERTY RUN_SERIAL TRUE)
set_property(TEST mitkSecurityMiddlewareTest PROPERTY RUN_SERIAL TRUE)

# Inject the source-tree path to the REST API spec markdown into the conformance
# test driver so it can verify the openapi.json <-> spec MD <-> handler triangle.
# Uses MITK_SOURCE_DIR (defined at the top-level CMake) so this works regardless
# of where the build tree lives.
target_compile_definitions(MitkRESTAPITestDriver PRIVATE
  MITK_REST_API_SPEC_MD_PATH="${MITK_SOURCE_DIR}/Documentation/Doxygen/2-UserManual/MITK_REST_API_Specification.md"
)
