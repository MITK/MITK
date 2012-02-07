SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  berryHelpContentView.cpp
  berryHelpEditor.cpp
  berryHelpEditorFindWidget.cpp
  berryHelpEditorInput.cpp
  berryHelpIndexView.cpp
  berryHelpPerspective.cpp
  berryHelpPluginActivator.cpp
  berryHelpSearchView.cpp
  berryHelpTopicChooser.cpp
  berryHelpWebView.cpp

  berryQHelpEngineConfiguration.cpp
  berryQHelpEngineWrapper.cpp
)

SET(CPP_FILES )

SET(MOC_H_FILES
  src/internal/berryHelpContentView.h
  src/internal/berryHelpEditor.h
  src/internal/berryHelpEditorFindWidget.h
  src/internal/berryHelpIndexView.h
  src/internal/berryHelpPerspective.h
  src/internal/berryHelpPluginActivator.h
  src/internal/berryHelpSearchView.h
  src/internal/berryHelpTopicChooser.h
  src/internal/berryHelpWebView.h

  src/internal/berryQHelpEngineConfiguration.h
  src/internal/berryQHelpEngineWrapper.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
  
  resources/help.png
)

SET(QRC_FILES
  resources/org_blueberry_ui_qt_help.qrc
)

SET(UI_FILES
  src/internal/berryHelpTopicChooser.ui
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
