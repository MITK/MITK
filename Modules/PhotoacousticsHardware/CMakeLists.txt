if(WIN32)

  option(MITK_USE_GALIL_HARDWARE "Enable support for OPOTEK Lasers" OFF)
  option(MITK_USE_OPHIR_PYRO_HARDWARE "Enable support for Ophir Pyroelectrical Sensors" OFF)

  if(MITK_USE_OPHIR_PYRO_HARDWARE)
    set(MITK_OPHIR_API_PATH "" CACHE PATH "Path to Ophir API lib.")
  endif()

  if(MITK_USE_GALIL_HARDWARE)
    set(MITK_GALIL_API_PATH "" CACHE PATH "Path to Galil API header files.")
    MITK_CREATE_MODULE(
      DEPENDS MitkIGT
      INCLUDE_DIRS PUBLIC "${MITK_GALIL_API_PATH}/include" "${MITK_OPHIR_API_PATH}"
      INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
      ADDITIONAL_LIBS "${MITK_GALIL_API_PATH}/lib/dynamic/x64/gclib.lib" "${MITK_GALIL_API_PATH}/lib/dynamic/x64/gclibo.lib" tinyxml "${MITK_OPHIR_API_PATH}/OphirPyroWrapper.lib"
      PACKAGE_DEPENDS tinyxml
    )
  endif()
endif()
