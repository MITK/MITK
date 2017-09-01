set(DCMQI_DIR ${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI-build)

find_path(DCMQI_INCLUDE_DIR
  NAMES DCMQI/DCMQI
  PATHS ${DCMQI_DIR}/../DCMQI ${MITK_EXTERNAL_PROJECT_PREFIX} ${CMAKE_PREFIX_PATH}
  PATH_SUFFIXES include include/dcmqi include/dcmqi/internal
)

set(DCMQI_INCLUDE_DIR "${DCMQI_DIR}/include" "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/include" "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/jsoncpp" ${DCMQI_DIR})


# Find all libraries, store debug and release separately
  # Find Release libraries
  find_library(DCMQI_LIBRARY_RELEASE
    dcmqi
    PATHS
    ${DCMQI_DIR}/bin
    ${DCMQI_DIR}/bin/Release
    ${DCMQI_DIR}/bin/RelWithDebInfo
    NO_DEFAULT_PATH
    )

  # Find Debug libraries
  find_library(DCMQI_LIBRARY_DEBUG
    dcmqi${DCMTK_CMAKE_DEBUG_POSTFIX}
    PATHS
    ${DCMQI_DIR}/bin
    ${DCMQI_DIR}/bin/Debug
    NO_DEFAULT_PATH
    )

  mark_as_advanced(DCMQI_LIBRARY_RELEASE)
  mark_as_advanced(DCMQI_LIBRARY_DEBUG)

  # Add libraries to variable according to build type
  set(DCMQI_LIBRARIES)

  if(DCMQI_LIBRARY_RELEASE)
    list(APPEND DCMQI_LIBRARIES optimized ${DCMQI_LIBRARY_RELEASE})
  endif()

  if(DCMQI_LIBRARY_DEBUG)
    list(APPEND DCMQI_LIBRARIES debug ${DCMQI_LIBRARY_DEBUG})
  endif()

find_package_handle_standard_args(DCMQI
  FOUND_VAR DCMQI_FOUND
  REQUIRED_VARS DCMQI_INCLUDE_DIR DCMQI_LIBRARIES
)
