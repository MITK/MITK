SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkDeformableRegistrationView.cpp
  QmitkDemonsRegistrationView.cpp

)

SET(UI_FILES
  src/internal/QmitkDeformableRegistrationViewControls.ui
  src/internal/QmitkDemonsRegistrationViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkDeformableRegistrationView.h
  src/internal/QmitkDemonsRegistrationView.h
)

SET(RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  resources/DeformableRegistration.xpm
)

SET(RES_FILES
  resources/QmitkDeformableRegistrationView.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

