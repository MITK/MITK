#Begin MicronTracker Hardware
option(MITK_USE_MICRON_TRACKER "Enable support for micron tracker hardware" OFF)

# only if MicronTracker is enabled
if(MITK_USE_MICRON_TRACKER)
  find_library(MITK_MICRON_TRACKER_LIB MTC DOC "Path which contains the MT2 library.")
  get_filename_component(MICRON_TRACKER_SDK_DIR ${MITK_MICRON_TRACKER_LIB} PATH)
  find_path(MITK_MICRON_TRACKER_INCLUDE_DIR MTC.h ${MICRON_TRACKER_SDK_DIR} DOC  "Include directory of the MT2.")
  find_path(MITK_MICRON_TRACKER_TEMP_DIR . DOC "Any temporary directory which can be used by the MicronTracker2.")
  find_path(MITK_MICRON_TRACKER_CALIBRATION_DIR BumbleBee_6400420.calib DOC "Path which contains the MT2 calibration file.")
  MITK_INSTALL(FILES ${MICRON_TRACKER_SDK_DIR}/MTC.dll  CONFIGURATIONS Release)
  MITK_INSTALL(FILES ${MICRON_TRACKER_SDK_DIR}/MTInterfaceDotNet.dll  CONFIGURATIONS Release)
  MITK_INSTALL(FILES ${MICRON_TRACKER_SDK_DIR}/PGRFlyCapture.dll  CONFIGURATIONS Release)
ENDIF(MITK_USE_MICRON_TRACKER)
#End MicronTracker Hardware


# only on Win32
if(WIN32)

  #Begin Ascension MicroBird Hardware
  option(MITK_USE_MICROBIRD_TRACKER "Enable support for Ascension MicroBird tracker hardware" OFF)
  if(MITK_USE_MICROBIRD_TRACKER)
   add_definitions(-DMITK_USE_MICROBIRD_TRACKER)
   find_library(MITK_USE_MICROBIRD_TRACKER_LIB PCIBird3)
   get_filename_component(MICROBIRD_TRACKER_API_DIR ${MITK_USE_MICROBIRD_TRACKER_LIB} PATH)
   find_path(MITK_USE_MICROBIRD_TRACKER_INCLUDE_DIR PCIBird3.h ${MICROBIRD_TRACKER_API_DIR})
  endif(MITK_USE_MICROBIRD_TRACKER)
  #End MicroBird Hardware

endif(WIN32)
