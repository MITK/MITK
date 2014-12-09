MITK_CREATE_MODULE(MitkTubeGraph
  DEPENDS Mitk SceneSerializationBase #Qmitk QmitkExt
  INCLUDE_DIRS Algorithms DataStructure Interactions IODataStructures Rendering
  PACKAGE_DEPENDS Boost
  WARNINGS_AS_ERRORS
)

# no complete tests yet
#IF(BUILD_TESTING)
  #ADD_SUBDIRECTORY(Testing)
#ENDIF(BUILD_TESTING)
