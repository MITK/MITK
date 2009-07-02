SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkSceneSerializationView.cpp

)

SET(UI_FILES
  src/internal/QmitkSceneSerializationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkSceneSerializationView.h
)

SET(RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
)

SET(RES_FILES
# uncomment the following line if you want to use Qt resources
#  resources/QmitkSceneSerializationView.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

