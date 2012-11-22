set(SRC_CPP_FILES
  IChangeText.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_example_gui_extensionpointdefinition_Activator.cpp
  ExtensionPointDefinition.cpp
  MinimalPerspective.cpp
  MinimalView.cpp
  ChangeTextDescriptor.cpp
  ChangeTextRegistry.cpp
  ExtensionPointDefinitionConstants.cpp
)

set(UI_FILES
  src/internal/MinimalViewControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_example_gui_extensionpointdefinition_Activator.h
  src/internal/ExtensionPointDefinition.h
  src/internal/MinimalPerspective.h
  src/internal/MinimalView.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
