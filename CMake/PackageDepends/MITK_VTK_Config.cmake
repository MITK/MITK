IF(NOT MITK_VTK_Config_INCLUDED)
  SET(MITK_VTK_Config_INCLUDED 1)
  FIND_PACKAGE(VTK REQUIRED)
  IF(VTK_FOUND AND NOT VTK_BUILD_SHARED_LIBS)
    MESSAGE(FATAL_ERROR "MITK only supports a VTK which was built with shared libraries. Turn on BUILD_SHARED_LIBS in your VTK config.")
  ENDIF()

  INCLUDE(${VTK_USE_FILE})
    LIST(APPEND ALL_INCLUDE_DIRECTORIES ${VTK_INCLUDE_DIRS})
  IF(VTK_FOR_MITK_LIBRARIES)
    LIST(APPEND ALL_LIBRARIES ${VTK_FOR_MITK_LIBRARIES})
  ELSE()
    # Libraries in mitk/Utilities may depend on VTK but
    # the VTK_FOR_MITK_LIBRARIES variable is not yet set.
    # Supply the VTK libraries manually
    LIST(APPEND ALL_LIBRARIES
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
  ENDIF()

ENDIF(NOT MITK_VTK_Config_INCLUDED)
