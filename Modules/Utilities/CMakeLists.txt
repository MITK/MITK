MITK_CREATE_MODULE(
  PACKAGE_DEPENDS
    PUBLIC Boost
    PRIVATE Qt5|Core
)

if (UNIX)
  set_target_properties(${MODULE_TARGET} PROPERTIES LINK_FLAGS "-lpthread")
endif (UNIX)
