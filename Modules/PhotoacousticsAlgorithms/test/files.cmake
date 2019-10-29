set(MODULE_TESTS
  # IMPORTANT: If you plan to deactivate / comment out a test please write a bug number to the commented out line of code.
  #
  #     Example: #mitkMyTest #this test is commented out because of bug 12345
  #
  # It is important that the bug is open and that the test will be activated again before the bug is closed. This assures that
  # no test is forgotten after it was commented out. If there is no bug for your current problem, please add a new one and
  # mark it as critical.
  ################## DISABLED TESTS #####################################################

  #mitkBandpassFilterTest.cpp #This test is disabled because of T25780
  #mitkBeamformingFilterTest.cpp #This test is disabled because of T25780

  ################# RUNNING TESTS #######################################################

  mitkPhotoacousticMotionCorrectionFilterTest.cpp
  mitkPAFilterServiceTest.cpp
  mitkCastToFloatImageFilterTest.cpp
  mitkCropImageFilterTest.cpp
  )
set(RESOURCE_FILES)
