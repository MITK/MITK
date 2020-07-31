if(NOT DEFINED DCMTK_dcmrt_LIBRARY OR DCMTK_dcmrt_LIBRARY)
  mitk_create_module(
    DEPENDS MitkSceneSerializationBase MitkDICOM
    PACKAGE_DEPENDS PUBLIC DCMTK
  )
  add_subdirectory(autoload/DICOMRTIO)
  add_subdirectory(test)
else()
  message("MITK RT Support disabled because the DCMTK dcmrt library not found")
endif()
