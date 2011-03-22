OPTION(MITK_TRACKING_DEVICES_TESTS_ENABLED "Enable support for testing tracking device hardware (this hardware must be connected to the system)" OFF)

IF(MITK_TRACKING_DEVICES_TESTS_ENABLED)
  OPTION(MITK_NDI_AURORA_CONNECTED "Turn on if an Aurora system is connected and should be tested." OFF)
  OPTION(MITK_NDI_POLARIS_CONNECTED "Turn on if a Polaris system is connected and should be tested." OFF)
  OPTION(MITK_CLARON_MICRONTRACKER_CONNECTED "Turn on if a MicronTracker system is connected and should be tested." OFF)
ENDIF(MITK_TRACKING_DEVICES_TESTS_ENABLED)

IF(MITK_NDI_AURORA_CONNECTED)
   set(MITK_NDI_AURORA_COM_PORT "1" CACHE STRING "Serial port where the system is connected.") 
ENDIF(MITK_NDI_AURORA_CONNECTED)

IF(MITK_NDI_POLARIS_CONNECTED)
   set(MITK_NDI_POLARIS_COM_PORT "1" CACHE STRING "Serial port where the system is connected.")
ENDIF(MITK_NDI_POLARIS_CONNECTED)