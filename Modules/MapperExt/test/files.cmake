# tests with no extra command line parameter
set(MODULE_TESTS
  # IMPORTANT: If you plan to deactivate / comment out a test please write a bug number to the commented out line of code.
  #
  #     Example: #mitkMyTest #this test is commented out because of bug 12345
  #
  # It is important that the bug is open and that the test will be activated again before the bug is closed. This assures that
  # no test is forgotten after it was commented out. If there is no bug for your current problem, please add a new one and
  # mark it as critical.

  ################## DISABLED TESTS #################################################

  ################# RUNNING TESTS ###################################################
)

if(MITK_ENABLE_RENDERING_TESTING)
set(MODULE_TESTS
  ${MODULE_TESTS}
  mitkSplineVtkMapper3DTest.cpp #new rendering test in CppUnit style
)
endif()

# test with image filename as an extra command line parameter
set(MODULE_IMAGE_TESTS
)

set(MODULE_SURFACE_TESTS
)

set(MODULE_POINTSET_TESTS
)

# image files
set(MODULE_TESTIMAGE
)

# surface files
set(MODULE_TESTSURFACE
)

# pointset files
set(MODULE_TESTPOINTSET
)


set(MODULE_CUSTOM_TESTS
)

set(RESOURCE_FILES)
