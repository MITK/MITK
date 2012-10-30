if(NOT MITK_VTK_Config_INCLUDED)
  set(MITK_VTK_Config_INCLUDED 1)
  find_package(VTK REQUIRED)
  if(VTK_FOUND AND NOT VTK_BUILD_SHARED_LIBS)
    message(FATAL_ERROR "MITK only supports a VTK which was built with shared libraries. Turn on BUILD_SHARED_LIBS in your VTK config.")
  endif()

  include(${VTK_USE_FILE})
  list(APPEND ALL_INCLUDE_DIRECTORIES ${VTK_INCLUDE_DIRS})
  list(APPEND ALL_LIBRARY_DIRS ${VTK_LIBRARY_DIRS})
  if(VTK_FOR_MITK_LIBRARIES)
    list(APPEND ALL_LIBRARIES ${VTK_FOR_MITK_LIBRARIES})
  else()
    # Libraries in mitk/Utilities may depend on VTK but
    # the VTK_FOR_MITK_LIBRARIES variable is not yet set.
    # Supply the VTK libraries manually
    list(APPEND ALL_LIBRARIES
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
  endif()

endif(NOT MITK_VTK_Config_INCLUDED)
