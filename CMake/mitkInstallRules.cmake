MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/mitk.ico )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/mitk.bmp )

#STATEMACHINE XML
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Core/Code/Interactions/StateMachine.xml )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Config/mitkLevelWindowPresets.xml )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Config/mitkRigidRegistrationPresets.xml )
MITK_INSTALL(FILES ${MITK_SOURCE_DIR}/Config/mitkRigidRegistrationTestPresets.xml )


if(WIN32)
  #DCMTK Dlls install target (shared libs on gcc only)
  if(MINGW AND DCMTK_ofstd_LIBRARY)
    set(_dcmtk_libs
        ${DCMTK_dcmdata_LIBRARY}
        ${DCMTK_dcmimgle_LIBRARY}
        ${DCMTK_dcmnet_LIBRARY}
        ${DCMTK_ofstd_LIBRARY}
       )
       
    foreach(_dcmtk_lib ${_dcmtk_libs})
      MITK_INSTALL(FILES ${_dcmtk_lib} )
    endforeach()
  endif()
  
  #BlueBerry
  # Since this file is also included from external projects, you
  # can only use variables which are made available through MITKConfig.cmake
  if(MITK_USE_BLUEBERRY)
    if(MINGW)
      MITK_INSTALL(FILES ${MITK_BINARY_DIR}/bin/plugins/liborg_blueberry_osgi.dll)
    else()
      if(NOT APPLE)
        MITK_INSTALL(FILES ${MITK_BINARY_DIR}/bin/plugins/debug/liborg_blueberry_osgi.dll CONFIGURATIONS Debug)
        MITK_INSTALL(FILES ${MITK_BINARY_DIR}/bin/plugins/release/liborg_blueberry_osgi.dll CONFIGURATIONS Release)
      endif(NOT APPLE)
    endif()
  endif()
  
  #MinGW dll
  if(MINGW)
    find_library(MINGW_RUNTIME_DLL "mingwm10.dll" HINTS ${CMAKE_FIND_ROOT_PATH}/sys-root/mingw/bin)
    if(MINGW_RUNTIME_DLL)
      MITK_INSTALL(FILES ${MINGW_RUNTIME_DLL} )
    else()
      message(SEND_ERROR "Could not find mingwm10.dll which is needed for a proper install")
    endif()
  
    find_library(MINGW_GCC_RUNTIME_DLL "libgcc_s_dw2-1.dll" HINTS ${CMAKE_FIND_ROOT_PATH}/sys-root/mingw/bin)
    if(MINGW_GCC_RUNTIME_DLL)
      MITK_INSTALL(FILES ${MINGW_GCC_RUNTIME_DLL} )
    else()
      message(SEND_ERROR "Could not find libgcc_s_dw2-1.dll which is needed for a proper install")
    endif()
  endif()

else()

  #DCMTK Dlls install target (shared libs on gcc only)
  if(DCMTK_ofstd_LIBRARY)
    set(_dcmtk_libs
        ${DCMTK_dcmdata_LIBRARY}
        ${DCMTK_dcmimgle_LIBRARY}
        ${DCMTK_dcmnet_LIBRARY}
        ${DCMTK_ofstd_LIBRARY}
    )
    foreach(_dcmtk_lib ${_dcmtk_libs})
      #MITK_INSTALL(FILES ${_dcmtk_lib} DESTINATION lib)
    endforeach()
  endif()
 
endif()
 

