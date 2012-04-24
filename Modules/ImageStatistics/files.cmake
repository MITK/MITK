set(CPP_FILES
  mitkImageStatisticsCalculator.cpp
  mitkPointSetStatisticsCalculator.cpp
  mitkPointSetDifferenceStatisticsCalculator.cpp
)

if( ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_LESS 5.8 )
  message(STATUS "Using VTK 5.8 classes from MITK respository")
  set(CPP_FILES ${CPP_FILES}
    vtkImageStencilRaster.cxx
    vtkLassoStencilSource.cxx
    )
endif( ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_LESS 5.8 )
