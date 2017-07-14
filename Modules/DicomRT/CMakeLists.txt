if(NOT DEFINED DCMTK_dcmrt_LIBRARY OR DCMTK_dcmrt_LIBRARY)
  mitk_create_module(
    DEPENDS MitkSegmentation MitkSceneSerializationBase MitkLegacyIO MitkDICOMReader MitkDICOMReaderServices
    PACKAGE_DEPENDS PUBLIC Qt5|Core DCMTK
  )
  add_subdirectory(test)
else()
  message("MITK DicomRT Support disabled because the DCMTK dcmrt library not found")
endif()
