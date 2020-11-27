mitk_create_module(ModelFit
  INCLUDE_DIRS
    PUBLIC ${MITK_BINARY_DIR}
    PRIVATE src/Common src/Functors src/Models src/TestingHelper
  DEPENDS
    PUBLIC MitkCore MitkSceneSerializationBase
    PRIVATE	MitkMultilabel
  PACKAGE_DEPENDS
    PUBLIC ITK|Optimizers
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

add_subdirectory(autoload/IO)
add_subdirectory(autoload/Models)
add_subdirectory(cmdapps)
