mitk_create_module(
  DEPENDS MitkCore MitkMatchPointRegistration
  TARGET_DEPENDS PRIVATE IsotropicWavelets
)

add_subdirectory(MiniApps)
