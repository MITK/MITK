IF(MITK_USE_DCMTK)

IF(NOT DCMTK_DIR)
  SET(DCMTK_DIR "$ENV{DCMTK_DIR}" CACHE PATH "Location of DCMTK")
  SET(DCMTK_DIR "$ENV{DCMTK_DIR}")
  IF(NOT DCMTK_DIR)
    MESSAGE(FATAL_ERROR "DCMTK_DIR not set. Cannot proceed.")
  ENDIF(NOT DCMTK_DIR)
ENDIF(NOT DCMTK_DIR)

FIND_PACKAGE(DCMTK)
    
IF(NOT DCMTK_FOUND)
  MESSAGE(SEND_ERROR "DCMTK development files not found.\n Please check variables (e.g. DCMTK_DIR) for include directories and libraries.\nYou may set environment variable DCMTK_DIR before pressing 'configure'")
ENDIF(NOT DCMTK_FOUND)

IF( NOT WIN32 )
  SET(MISSING_LIBS_REQUIRED_BY_DCMTK wrap tiff z)
ENDIF( NOT WIN32 )

SET(QT_USE_QTSQL 1)

IF(EXISTS ${DCMTK_config_INCLUDE_DIR}/osconfig.h)

FILE(READ ${DCMTK_config_INCLUDE_DIR}/osconfig.h _osconfig_h)
  IF(NOT _osconfig_h MATCHES "PACKAGE_VERSION_NUMBER \"354\"")
  # MESSAGE(STATUS "Found DCMTK newer that 3.5.4 ...")
  SET(MITK_USE_DCMTK_NEWER_THAN_3_5_4 TRUE)
  # assume the new oflog library is located next to the others
  # this can be removed if FindDCMTK is adapted for 3.5.5 
  GET_FILENAME_COMPONENT(_DCMTK_lib_dir ${DCMTK_ofstd_LIBRARY} PATH)
  FIND_LIBRARY(DCMTK_oflog_LIBRARY oflog ${_DCMTK_lib_dir} )
  LIST(APPEND DCMTK_LIBRARIES ${DCMTK_oflog_LIBRARY})
  ENDIF(NOT _osconfig_h MATCHES "PACKAGE_VERSION_NUMBER \"354\"")
ENDIF(EXISTS ${DCMTK_config_INCLUDE_DIR}/osconfig.h)

#
# Usually all code should be adapted to DCMTK 3.6
# If necessary you could configure the MITK_USE_DCMTK_NEWER_THAN_3_5_4 variable
# to configure a header file for ifdefs:
# CONFIGURE_FILE( mitkDCMTKConfig.h.in mitkDCMTKConfig.h )

LIST(APPEND ALL_INCLUDE_DIRECTORIES ${DCMTK_INCLUDE_DIR} ${DCMTK_DIR}/include)
LIST(APPEND ALL_LIBRARIES ${DCMTK_LIBRARIES} ${MISSING_LIBS_REQUIRED_BY_DCMTK})
#link_directories()

ENDIF(MITK_USE_DCMTK)

