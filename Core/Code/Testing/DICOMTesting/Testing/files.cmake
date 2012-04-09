
# tests with no extra command line parameter
set(MODULE_CUSTOM_TESTS
  mitkDICOMTestingSanityTest.cpp
)

# this shouldn't be necessary if this variable
# would actually be a parameter of the MITK_CREATE_MODULE_TESTS
# macro. See bug #10592
set(TEST_CPP_FILES "")

