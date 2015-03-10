if( NOT WIN32 AND NOT APPLE )
  set(MISSING_LIBS_REQUIRED_BY_DCMTK wrap tiff z)
endif( NOT WIN32 AND NOT APPLE )

set(QT_USE_QTSQL 1)

if(EXISTS ${DCMTK_config_INCLUDE_DIR}/osconfig.h)

  file(READ ${DCMTK_config_INCLUDE_DIR}/osconfig.h _osconfig_h)
  if(NOT _osconfig_h MATCHES "PACKAGE_VERSION_NUMBER \"354\"")
    # message(STATUS "Found DCMTK newer that 3.5.4 ...")
    set(MITK_USE_DCMTK_NEWER_THAN_3_5_4 TRUE)
    # assume the new oflog library is located next to the others
    # this can be removed if FindDCMTK is adapted for 3.5.5
    # treat Debug and Release separately
    get_filename_component(_DCMTK_lib_dir_release ${DCMTK_ofstd_LIBRARY_RELEASE} PATH)
    get_filename_component(_DCMTK_lib_dir_debug ${DCMTK_ofstd_LIBRARY_DEBUG} PATH)
    set(DCMTK_oflog_LIBRARY_RELEASE )
    set(DCMTK_oflog_LIBRARY_DEBUG )
    if(_DCMTK_lib_dir_release)
      find_library(DCMTK_oflog_LIBRARY_RELEASE oflog ${_DCMTK_lib_dir_release} )
      list(APPEND DCMTK_LIBRARIES optimized ${DCMTK_oflog_LIBRARY_RELEASE} )
    endif()
    if(_DCMTK_lib_dir_debug)
      find_library(DCMTK_oflog_LIBRARY_DEBUG oflog ${_DCMTK_lib_dir_debug} )
      list(APPEND DCMTK_LIBRARIES debug ${DCMTK_oflog_LIBRARY_DEBUG} )
    endif()
  endif()
endif()

#
# Usually all code should be adapted to DCMTK 3.6
# If necessary you could configure the MITK_USE_DCMTK_NEWER_THAN_3_5_4 variable
# to configure a header file for ifdefs:
# configure_file( mitkDCMTKConfig.h.in mitkDCMTKConfig.h )

list(APPEND ALL_INCLUDE_DIRECTORIES ${DCMTK_INCLUDE_DIR})
list(APPEND ALL_LIBRARIES ${DCMTK_LIBRARIES} ${MISSING_LIBS_REQUIRED_BY_DCMTK})

