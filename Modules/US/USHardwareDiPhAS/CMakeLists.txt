if(WIN32)

  OPTION(MITK_USE_US_DiPhAS_SDK "Enable support for DiPhAS API devices" OFF)

  if(MITK_USE_US_DiPhAS_SDK)

    SET(MITK_US_DiPhAS_SDK_PATH "" CACHE PATH "Path to DiPhAS SDK header files.")

    MITK_CREATE_MODULE(
      DEPENDS MitkUS MitkPhotoacousticsHardware
      INCLUDE_DIRS PUBLIC "${MITK_US_DiPhAS_SDK_PATH}/Debug" "${MITK_US_DiPhAS_SDK_PATH}/Release"
      INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
      ADDITIONAL_LIBS "${MITK_US_DiPhAS_SDK_PATH}/Debug/Framework.IBMT.US.CWrapper.lib" "${MITK_US_DiPhAS_SDK_PATH}/Release/Framework.IBMT.US.CWrapper.lib"
      AUTOLOAD_WITH MitkUS
    )

  endif()

endif()
