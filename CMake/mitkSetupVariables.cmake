if(MITK_BUILD_ALL_PLUGINS)
  set(MITK_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
endif()

set(LIBPOSTFIX "")

# MITK_VERSION
set(MITK_VERSION_MAJOR "2013")
set(MITK_VERSION_MINOR "09")
set(MITK_VERSION_PATCH "99")
set(MITK_VERSION_STRING "${MITK_VERSION_MAJOR}.${MITK_VERSION_MINOR}.${MITK_VERSION_PATCH}")
if(MITK_VERSION_PATCH STREQUAL "99")
  set(MITK_VERSION_STRING "${MITK_VERSION_STRING}-${MITK_REVISION_SHORTID}")
endif()

# Needed early on for redirecting the BlueBerry documentation output dir
set(MITK_DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Documentation/Doxygen CACHE PATH
    "Output directory for doxygen generated documentation." )

#-----------------------------------
# Configuration of module system
#-----------------------------------
set(MODULES_CONF_DIRNAME modulesConf)
set(MODULES_CONF_DIRS ${MITK_BINARY_DIR}/${MODULES_CONF_DIRNAME})

if(NOT UNIX AND NOT MINGW)
  set(MITK_WIN32_FORCE_STATIC "STATIC" CACHE INTERNAL "Use this variable to always build static libraries on non-unix platforms")
endif()

# build the MITK_INCLUDE_DIRS variable
set(MITK_INCLUDE_DIRS
    ${ITK_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR} # contains mitkConfig.h and similar files
    ${MODULES_CONF_DIRS} # contains module *Exports.h files
   )
set(CORE_DIRECTORIES Common DataManagement Algorithms IO Rendering Interactions Controllers Service)
foreach(d ${CORE_DIRECTORIES})
  list(APPEND MITK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Core/Code/${d})
endforeach()
#list(APPEND MITK_INCLUDE_DIRS
     #${ITK_INCLUDE_DIRS}
     #${VTK_INCLUDE_DIRS}
#    )

foreach(d Utilities Utilities/ipPic Utilities/mbilog)
  list(APPEND MITK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${d})
endforeach()
list(APPEND MITK_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/Utilities/mbilog)

if(WIN32)
  list(APPEND MITK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipPic/win32)
endif()

# additional include dirs variables
set(IPSEGMENTATION_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipSegmentation)

# variables containing librariy names
set(MITK_CORE_LIBRARIES Mitk)
set(VTK_FOR_MITK_LIBRARIES
    vtkGraphics
    vtkCommon
    vtkFiltering
    vtkGraphics
    vtkHybrid
    vtkImaging
    vtkIO
    vtkParallel
    vtkRendering
    vtkVolumeRendering
    vtkWidgets
    ${VTK_JPEG_LIBRARIES}
    ${VTK_PNG_LIBRARIES}
    ${VTK_ZLIB_LIBRARIES}
    ${VTK_EXPAT_LIBRARIES}
    ${VTK_FREETYPE_LIBRARIES}
   )
# TODO: maybe solve this with lib depends mechanism of CMake
set(UTIL_FOR_MITK_LIBRARIES mbilog)
set(LIBRARIES_FOR_MITK_CORE
    ${UTIL_FOR_MITK_LIBRARIES}
    ${VTK_FOR_MITK_LIBRARIES}
    ${ITK_LIBRARIES}
   )
set(MITK_LIBRARIES
    ${MITK_CORE_LIBRARIES}
    ${LIBRARIES_FOR_MITK_CORE}
   )

# variables used in CMake macros which are called from external projects
set(MITK_VTK_LIBRARY_DIRS ${VTK_LIBRARY_DIRS})
set(MITK_ITK_LIBRARY_DIRS ${ITK_LIBRARY_DIRS})

# variables containing link directories
set(MITK_LIBRARY_DIRS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(MITK_LINK_DIRECTORIES
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ${ITK_LIBRARY_DIRS}
    ${VTK_LIBRARY_DIRS}
    ${GDCM_LIBRARY_DIRS})

# Qt support
if(MITK_USE_QT)

  if(DESIRED_QT_VERSION MATCHES 4)
    find_package(Qt4 REQUIRED)

    set(QMITK_INCLUDE_DIRS
        ${MITK_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Modules/Qmitk
        ${PROJECT_BINARY_DIR}/Modules/Qmitk
       )

    set(QMITK_LIBRARIES Qmitk ${MITK_LIBRARIES})
    set(QMITK_LINK_DIRECTORIES ${MITK_LINK_DIRECTORIES})
  elseif(DESIRED_QT_VERSION MATCHES 5)
    find_package(Qt5Core REQUIRED) # at least Core required

    set(QMITK_INCLUDE_DIRS
        ${MITK_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Modules/QmlMitk
        ${PROJECT_BINARY_DIR}/Modules/QmlMitk
       )

    set(QMITK_LIBRARIES QmlMitk ${MITK_LIBRARIES})
    set(QMITK_LINK_DIRECTORIES ${MITK_LINK_DIRECTORIES})
  endif()
endif()

if(MITK_BUILD_ALL_PLUGINS)
  set(MITK_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
endif()

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

# separate processing of the COMPOSITE list to avoid its concatenation to to global list
_create_type_seq(${MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES}
                 MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ
                 MITK_ACCESSBYITK_COMPOSITE_TYPES_DIMN_SEQ)

set(MITK_ACCESSBYITK_DIMENSIONS_SEQ )
string(REPLACE "," ";" _dimensions "${MITK_ACCESSBYITK_DIMENSIONS}")
foreach(_dimension ${_dimensions})
  set(MITK_ACCESSBYITK_DIMENSIONS_SEQ "${MITK_ACCESSBYITK_DIMENSIONS_SEQ}(${_dimension})")
endforeach()

