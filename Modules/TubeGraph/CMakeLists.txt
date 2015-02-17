set(disable_module 0)
if(GCC_VERSION VERSION_GREATER 0 AND
   GCC_VERSION VERSION_LESS 4.7)
  # The Boost Graph library at least up to version 1.57 does not
  # compile with gcc < 4.7 and -std=c++0x, see
  # http://stackoverflow.com/questions/25395805/compile-error-with-boost-graph-1-56-0-and-g-4-6-4
  set(disable_module 1)
endif()

if(NOT disable_module)

MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Algorithms src/DataStructure src/Interactions src/Rendering src/IO
  DEPENDS MitkSceneSerializationBase
  PACKAGE_DEPENDS PUBLIC Boost
  #WARNINGS_AS_ERRORS
)

#add_subdirectory(test)

endif()
