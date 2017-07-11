MITK_CREATE_MODULE(
  DEPENDS MitkUtilities
  PACKAGE_DEPENDS PUBLIC Boost
)

if (UNIX)
  set_target_properties(${MODULE_TARGET} PROPERTIES LINK_FLAGS "-lpthread")
endif (UNIX)
