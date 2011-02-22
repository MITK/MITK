if(MITK_BUILD_ALL_PLUGINS)
  set(MITK_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
endif()

set(LIBPOSTFIX "")

# MITK_VERSION
set(MITK_VERSION_MAJOR "0")
set(MITK_VERSION_MINOR "15")
set(MITK_VERSION_PATCH "1")
set(MITK_VERSION_STRING "${MITK_VERSION_MAJOR}.${MITK_VERSION_MINOR}.${MITK_VERSION_PATCH}")

if(NOT UNIX AND NOT MINGW)
  set(MITK_WIN32_FORCE_STATIC "STATIC" CACHE INTERNAL "Use this variable to always build static libraries on non-unix platforms")
endif()

# build the MITK_INCLUDE_DIRS variable
set(MITK_INCLUDE_DIRS ${PROJECT_BINARY_DIR})
set(CORE_DIRECTORIES DataManagement Algorithms IO Rendering Interactions Controllers)
foreach(d ${CORE_DIRECTORIES})
  list(APPEND MITK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Core/Code/${d})
endforeach()
#list(APPEND MITK_INCLUDE_DIRS  
     #${ITK_INCLUDE_DIRS}
     #${VTK_INCLUDE_DIRS}
#    )
     
foreach(d Utilities Utilities/ipPic Utilities/IIL4MITK Utilities/pic2vtk Utilities/tinyxml Utilities/mbilog)
  list(APPEND MITK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
endforeach()

if(WIN32)
  list(APPEND MITK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipPic/win32)
endif()

# additional include dirs variables
set(ANN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ann/include)
set(IPSEGMENTATION_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipSegmentation)

# variables containing librariy names
set(MITK_CORE_LIBRARIES mitkCore)
set(VTK_FOR_MITK_LIBRARIES
    vtkGraphics
    vtkCommon
    vtkFiltering
    vtkftgl
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
set(UTIL_FOR_MITK_LIBRARIES mitkIpPic mitkIpFunc mbilog)
set(LIBRARIES_FOR_MITK_CORE
    ${UTIL_FOR_MITK_LIBRARIES}
    ${VTK_FOR_MITK_LIBRARIES}
    ${ITK_LIBRARIES}
   )
set(MITK_LIBRARIES
    ${MITK_CORE_LIBRARIES}
    ${LIBRARIES_FOR_MITK_CORE}
    pic2vtk
    IIL4MITK
    ipSegmentation
    ann 
   )

# variables containing link directories
set(MITK_LIBRARY_DIRS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})                                                                                                              
set(MITK_LINK_DIRECTORIES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${ITK_LIBRARY_DIRS} ${VTK_LIBRARY_DIRS})

# Qt support
if(MITK_USE_QT)
  find_package(Qt4 REQUIRED)

  set(QMITK_INCLUDE_DIRS 
      ${MITK_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/Qmitk
      ${PROJECT_BINARY_DIR}/CoreUI/Qmitk
     )
     
  foreach(d QmitkApplicationBase QmitkModels QmitkPropertyObservers) 
    list(APPEND QMITK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/Qmitk/${d})
  endforeach()
  list(APPEND QMITK_INCLUDE_DIRS ${QWT_INCLUDE_DIR})
  
  set(QMITK_LIBRARIES Qmitk ${MITK_LIBRARIES} ${QT_LIBRARIES})
  set(QMITK_LINK_DIRECTORIES ${MITK_LINK_DIRECTORIES} ${QT_LIBRARY_DIR})
endif()

if(MITK_BUILD_ALL_PLUGINS)
  set(MITK_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
endif()


