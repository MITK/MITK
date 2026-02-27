MITK_CREATE_MODULE(DEPENDS MitkQtWidgets)

if(MODULE_IS_ENABLED AND APPLE)
  target_compile_options(${MODULE_TARGET} PRIVATE -Wno-error=unused-private-field)
endif()
