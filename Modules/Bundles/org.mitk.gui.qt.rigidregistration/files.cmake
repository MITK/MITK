SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkRigidRegistrationView.cpp
  QmitkLoadPresetDialog.cpp
  QmitkRigidRegistrationSelectorView.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkRigidRegistrationViewControls.ui
  src/internal/QmitkRigidRegistrationSelector.ui
)

SET(MOC_H_FILES
  src/internal/QmitkRigidRegistrationView.h
  src/internal/QmitkLoadPresetDialog.h
  src/internal/QmitkRigidRegistrationSelectorView.h
  src/internal/mitkPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  resources/RigidRegistration.xpm
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkRigidRegistrationView.qrc
)

SET(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

