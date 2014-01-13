# CREATE MODULE HERE
IF (NOT DEFINED DCMTK_dcmrt_LIBRARY OR DCMTK_dcmrt_LIBRARY)
MITK_CREATE_MODULE(mitkDicomRT
  INCLUDE_DIRS DataStructures
  DEPENDS Mitk Segmentation SceneSerializationBase
  PACKAGE_DEPENDS DCMTK
  QT_MODULE
)
ADD_SUBDIRECTORY(Testing)
ELSE()
MESSAGE("MITK DicomRT Support disabled because the DCMTK dcmrt library not found")
ENDIF()
