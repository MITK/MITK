# CREATE MODULE HERE
IF (NOT DEFINED DCMTK_dcmrt_LIBRARY OR DCMTK_dcmrt_LIBRARY)
MITK_CREATE_MODULE(
  INCLUDE_DIRS DataStructures Rendering
  DEPENDS MitkSegmentation MitkSceneSerializationBase
  PACKAGE_DEPENDS DCMTK Qt4|QtCore
)
ADD_SUBDIRECTORY(Testing)
ELSE()
MESSAGE("MITK DicomRT Support disabled because the DCMTK dcmrt library not found")
ENDIF()
