if(NOT DEFINED DCMTK_dcmrt_LIBRARY OR DCMTK_dcmrt_LIBRARY)
  mitk_create_module(
    DEPENDS MitkSceneSerializationBase MitkDICOMReader MitkDICOMReaderServices
    PACKAGE_DEPENDS PUBLIC DCMTK
  )
  add_subdirectory(autoload/IO)
  add_subdirectory(test)
else()
  message("MITK DicomRT Support disabled because the DCMTK dcmrt library not found")
endif()
