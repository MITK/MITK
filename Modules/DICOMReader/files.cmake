file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkDICOMFileReader.cpp
  mitkDICOMGDCMTagScanner.cpp
  mitkDICOMImageBlockDescriptor.cpp
  mitkDICOMITKSeriesGDCMReader.cpp
  mitkDICOMDatasetSorter.cpp
  mitkDICOMTagBasedSorter.cpp
  mitkDICOMGDCMImageFrameInfo.cpp
  mitkDICOMImageFrameInfo.cpp
  mitkDICOMSortCriterion.cpp
  mitkDICOMSortByTag.cpp
  mitkITKDICOMSeriesReaderHelper.cpp
  mitkEquiDistantBlocksSorter.cpp
  mitkNormalDirectionConsistencySorter.cpp
  mitkSortByImagePositionPatient.cpp
  mitkGantryTiltInformation.cpp
  mitkClassicDICOMSeriesReader.cpp
  mitkThreeDnTDICOMSeriesReader.cpp
  mitkDICOMTag.cpp
  mitkDICOMTagHelper.cpp
  mitkDICOMTagCache.cpp
  mitkDICOMEnums.cpp
  mitkDICOMReaderConfigurator.cpp
  mitkDICOMFileReaderSelector.cpp
  mitkIDICOMTagsOfInterest.cpp
  mitkDICOMProperty.cpp
)

set(RESOURCE_FILES
  configurations/3D/classicreader.xml
  configurations/3D/imageposition.xml
  configurations/3D/imageposition_byacquisition.xml
  configurations/3D/instancenumber.xml
  configurations/3D/instancenumber_soft.xml
  configurations/3D/slicelocation.xml

  configurations/3DnT/classicreader.xml
  configurations/3DnT/imageposition_byacquisition.xml
  configurations/3DnT/imageposition_bytriggertime.xml
)
