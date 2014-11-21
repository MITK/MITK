#-----------------------------------
# Basic MITK CMake variables
#-----------------------------------

# MITK_VERSION
set(MITK_VERSION_MAJOR "2014")
set(MITK_VERSION_MINOR "10")
set(MITK_VERSION_PATCH "99")
set(MITK_VERSION_STRING "${MITK_VERSION_MAJOR}.${MITK_VERSION_MINOR}.${MITK_VERSION_PATCH}")
if(MITK_VERSION_PATCH STREQUAL "99")
  set(MITK_VERSION_STRING "${MITK_VERSION_STRING}-${MITK_REVISION_SHORTID}")
endif()

# Needed early on for redirecting the BlueBerry documentation output dir
set(MITK_DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Documentation/Doxygen CACHE PATH
    "Output directory for doxygen generated documentation." )

if(NOT UNIX AND NOT MINGW)
  set(MITK_WIN32_FORCE_STATIC "STATIC" CACHE INTERNAL "Use this variable to always build static libraries on non-unix platforms")
endif()

if(MITK_BUILD_ALL_PLUGINS)
  set(MITK_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
endif()


#-----------------------------------
# Find external dependencies
#-----------------------------------

# Qt support
if(MITK_USE_QT)
  if(DESIRED_QT_VERSION MATCHES 4)
    find_package(Qt4 ${MITK_QT4_MINIMUM_VERSION} REQUIRED)
  elseif(DESIRED_QT_VERSION MATCHES 5)
    find_package(Qt5Core ${MITK_QT5_MINIMUM_VERSION} REQUIRED) # at least Core required
  endif()
endif()


#-----------------------------------
# Configuration of module system
#-----------------------------------
set(MODULES_CONF_DIRNAME modulesConf)
set(MODULES_CONF_DIRS ${MITK_BINARY_DIR}/${MODULES_CONF_DIRNAME})

# Configure module naming conventions
set(MITK_MODULE_NAME_REGEX_MATCH "^[A-Z].*$")
set(MITK_MODULE_NAME_REGEX_NOT_MATCH "^[Mm][Ii][Tt][Kk].*$")
set(MITK_MODULE_NAME_PREFIX "Mitk")
set(MITK_MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME 1)

#-----------------------------------
# Configuration of the buid-in MITK pixel types
#-----------------------------------

# create a list of types for template instantiations of itk image access functions
function(_create_type_seq TYPES seq_var seqdim_var)
  set(_seq )
  set(_seq_dim )
  string(REPLACE "," ";" _pixeltypes "${TYPES}")
  foreach(_pixeltype ${_pixeltypes})
    set(_seq "${_seq}(${_pixeltype})")
    set(_seq_dim "${_seq_dim}((${_pixeltype},dim))")
  endforeach()
  set(${seq_var} "${_seq}" PARENT_SCOPE)
  set(${seqdim_var} "${_seq_dim}" PARENT_SCOPE)
endfunction()

set(MITK_ACCESSBYITK_PIXEL_TYPES )
set(MITK_ACCESSBYITK_PIXEL_TYPES_SEQ )
set(MITK_ACCESSBYITK_TYPES_DIMN_SEQ )
# concatenate only the simple pixel types to the MITK_ACCESSBYITK_PIXEL_TYPE_SEQ list
# see Bug 12682 for detailed information
foreach(_type INTEGRAL FLOATING)
  set(_typelist "${MITK_ACCESSBYITK_${_type}_PIXEL_TYPES}")
  if(_typelist)
    if(MITK_ACCESSBYITK_PIXEL_TYPES)
      set(MITK_ACCESSBYITK_PIXEL_TYPES "${MITK_ACCESSBYITK_PIXEL_TYPES},${_typelist}")
    else()
      set(MITK_ACCESSBYITK_PIXEL_TYPES "${_typelist}")
    endif()
  endif()

  _create_type_seq("${_typelist}"
                   MITK_ACCESSBYITK_${_type}_PIXEL_TYPES_SEQ
                   MITK_ACCESSBYITK_${_type}_TYPES_DIMN_SEQ)
  set(MITK_ACCESSBYITK_PIXEL_TYPES_SEQ "${MITK_ACCESSBYITK_PIXEL_TYPES_SEQ}${MITK_ACCESSBYITK_${_type}_PIXEL_TYPES_SEQ}")
  set(MITK_ACCESSBYITK_TYPES_DIMN_SEQ "${MITK_ACCESSBYITK_TYPES_DIMN_SEQ}${MITK_ACCESSBYITK_${_type}_TYPES_DIMN_SEQ}")
endforeach()

# separate processing of the COMPOSITE list to avoid its concatenation to the global list
_create_type_seq(${MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES}
                 MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ
                 MITK_ACCESSBYITK_COMPOSITE_TYPES_DIMN_SEQ)

# separate processing of the VECTOR list to avoid its concatenation to the global list
_create_type_seq(${MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES}
                 MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ
                 MITK_ACCESSBYITK_VECTOR_TYPES_DIMN_SEQ)

set(MITK_ACCESSBYITK_DIMENSIONS_SEQ )
string(REPLACE "," ";" _dimensions "${MITK_ACCESSBYITK_DIMENSIONS}")
foreach(_dimension ${_dimensions})
  set(MITK_ACCESSBYITK_DIMENSIONS_SEQ "${MITK_ACCESSBYITK_DIMENSIONS_SEQ}(${_dimension})")
endforeach()
