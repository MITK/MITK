set(CPP_FILES
  mitkImageStatisticsCalculator.cpp
  mitkPointSetStatisticsCalculator.cpp
  mitkPointSetDifferenceStatisticsCalculator.cpp
)

set(H_FILES
  mitkImageStatisticsCalculator.h
  mitkPointSetDifferenceStatisticsCalculator.h
  mitkPointSetStatisticsCalculator.h
  mitkStatisticsImageFilter.h
  mitkExtendedLabelStatisticsImageFilter.h
)

if( ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_LESS 5.8 )
  message(STATUS "Using VTK 5.8 classes from MITK respository")
  set(CPP_FILES ${CPP_FILES}
    vtkImageStencilRaster.cxx
    vtkLassoStencilSource.cxx
    )
endif( ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_LESS 5.8 )
