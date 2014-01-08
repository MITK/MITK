foreach(stepNo RANGE 1 8)
  mitk_create_executable(Step${stepNo}
    FILES_CMAKE Step${stepNo}.cmake
    DEPENDS QmitkExt
    QT4_MODULES QtGui
    )
endforeach()

# adding additional example for mitk itk and vtk interaction
mitk_create_executable(mitkWithITKAndVTK
  DEPENDS Mitk IpPicSupport
  FILES_CMAKE mitkWithITKAndVTK.cmake
  )
