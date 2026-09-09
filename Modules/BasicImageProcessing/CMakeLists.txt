mitk_create_module(
  DEPENDS MitkCore MitkMatchPointRegistration
  PACKAGE_DEPENDS PRIVATE ITK|FFT
  TARGET_DEPENDS PRIVATE IsotropicWavelets
)

add_subdirectory(cmdapps)
