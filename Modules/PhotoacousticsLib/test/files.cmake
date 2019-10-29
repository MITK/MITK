set(MODULE_TESTS
  # IMPORTANT: If you plan to deactivate / comment out a test please write a bug number to the commented out line of code.
  #
  #     Example: #mitkMyTest #this test is commented out because of bug 12345
  #
  # It is important that the bug is open and that the test will be activated again before the bug is closed. This assures that
  # no test is forgotten after it was commented out. If there is no bug for your current problem, please add a new one and
  # mark it as critical.

  ################## ON THE FENCE TESTS #################################################
  # none

  ################## DISABLED TESTS #####################################################
  # mitkMCThreadHandlerTest.cpp #Timing issue on VMs
  # mitkPhotoacousticIOTest.cpp #Not suitable for large scale testing as it performs IO actions.
  ################# RUNNING TESTS #######################################################

  mitkSlicedVolumeGeneratorTest.cpp
  mitkPhotoacousticTissueGeneratorTest.cpp
  mitkPhotoacousticVectorTest.cpp
  mitkPhotoacoustic3dVolumeTest.cpp
  mitkPhotoacousticVolumeTest.cpp
  mitkPhotoacousticVesselTreeTest.cpp
  mitkMcxyzXmlTest.cpp
  mitkPhotoacousticComposedVolumeTest.cpp
  mitkPhotoacousticNoiseGeneratorTest.cpp
  mitkSimulationBatchGeneratorTest.cpp
  mitkPropertyCalculatorTest.cpp
  mitkSpectralUnmixingTest.cpp
  mitkPhotoacousticVesselMeanderStrategyTest.cpp
  mitkPhotoacousticVesselTest.cpp
)

set(RESOURCE_FILES
  pointsource.xml
  circlesource.xml
  rectanglesource.xml
  twopointsources.xml
  allsources.xml
)
