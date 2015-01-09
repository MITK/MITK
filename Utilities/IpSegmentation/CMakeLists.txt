set(module_options C_MODULE NO_INIT GCC_DEFAULT_VISIBILITY)
if(MITK_WIN32_FORCE_STATIC)
  list(APPEND module_options FORCE_STATIC)
endif()

MITK_CREATE_MODULE(
  DEPENDS MitkIpPic MitkIpFunc
  PACKAGE_DEPENDS ANN
  ${module_options}
)
