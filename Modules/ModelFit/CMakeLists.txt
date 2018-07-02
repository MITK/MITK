MITK_CREATE_MODULE(ModelFit
  INCLUDE_DIRS
    PUBLIC ${MITK_BINARY_DIR}
    PRIVATE src/Common src/Functors src/Models src/TestingHelper
  DEPENDS
    PUBLIC MitkCore MitkSceneSerializationBase
    PRIVATE	MitkMultilabel
  PACKAGE_DEPENDS
    PUBLIC ITK|ITKOptimizers
    PRIVATE Boost
)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(test)
endif(BUILD_TESTING)

ADD_SUBDIRECTORY(autoload/IO)
ADD_SUBDIRECTORY(autoload/Models)
ADD_SUBDIRECTORY(cmdapps)
