#Begin MicronTracker Hardware
option(MITK_USE_MICRON_TRACKER "Enable support for micron tracker hardware" OFF)
#Begin Optitrack Hardware
option(MITK_USE_OPTITRACK_TRACKER "Enable support for Optitrack tracker hardware" OFF)

option(MITK_USE_POLHEMUS_TRACKER "Enable support for Polhemus tracker hardware" OFF)

# only if MicronTracker is enabled
if(MITK_USE_MICRON_TRACKER)
  find_library(MITK_MICRON_TRACKER_LIB MTC DOC "Path which contains the MT2 library.")
  get_filename_component(MICRON_TRACKER_SDK_DIR ${MITK_MICRON_TRACKER_LIB} PATH)
  find_path(MITK_MICRON_TRACKER_INCLUDE_DIR MTC.h ${MICRON_TRACKER_SDK_DIR} DOC  "Include directory of the MT2.")
  MITK_INSTALL(FILES ${MICRON_TRACKER_SDK_DIR}/MTC.dll  CONFIGURATIONS Release)
  MITK_INSTALL(FILES ${MICRON_TRACKER_SDK_DIR}/MTInterfaceDotNet.dll  CONFIGURATIONS Release)
  MITK_INSTALL(FILES ${MICRON_TRACKER_SDK_DIR}/PGRFlyCapture.dll  CONFIGURATIONS Release)
ENDIF(MITK_USE_MICRON_TRACKER)
#End MicronTracker Hardware

#only if Optitrack is enabled
if(MITK_USE_OPTITRACK_TRACKER)
  find_library(MITK_OPTITRACK_TRACKER_LIB NPTrackingTools DOC "Path which contains the Optitrack library. Please choose 32/64 bit version depending on your build.")
  find_path(MITK_OPTITRACK_TRACKER_INCLUDE_DIR NPTrackinTools.h DOC  "Include directory of the Optitrack library.")
  find_path(MITK_OPTITRACK_TRACKER_LIB_DIR NPTrackingTools.dll)
  MITK_INSTALL(FILES ${MITK_OPTITRACK_TRACKER_LIB_DIR}/NPTrackingTools.dll CONFIGURATIONS Release)
  MITK_INSTALL(FILES ${MITK_OPTITRACK_TRACKER_LIB_DIR}/NPTrackingToolsx64.dll CONFIGURATIONS Release)
ENDIF(MITK_USE_OPTITRACK_TRACKER)
#End Optitrack Hardware

if(MITK_USE_POLHEMUS_TRACKER)
  find_library(MITK_POLHEMUS_TRACKER_LIB PDI DOC "Path which contains the Polhemus library.")
  find_path(MITK_POLHEMUS_TRACKER_INCLUDE_DIR PDI.h DOC  "Include directory of the Polhemus library.")
ENDIF(MITK_USE_POLHEMUS_TRACKER)


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
