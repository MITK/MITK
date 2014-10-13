# CREATE MODULE HERE
IF (NOT DEFINED DCMTK_dcmrt_LIBRARY OR DCMTK_dcmrt_LIBRARY)
MITK_CREATE_MODULE(
  INCLUDE_DIRS DataStructures
  DEPENDS MitkSegmentation MitkSceneSerializationBase
  PACKAGE_DEPENDS DCMTK
)
ADD_SUBDIRECTORY(Testing)
ELSE()
MESSAGE("MITK DicomRT Support disabled because the DCMTK dcmrt library not found")
ENDIF()
