SET(CPP_FILES
  mitkImageStatisticsCalculator.cpp
  mitkPointSetStatisticsCalculator.cpp
)

IF ( ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_LESS 5.8 )
  MESSAGE(STATUS "Using VTK 5.8 classes from MITK respository")
  SET(CPP_FILES ${CPP_FILES}
    vtkImageStencilRaster.cxx
    vtkLassoStencilSource.cxx
    )
ENDIF ( ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_LESS 5.8 )