SET(SRC_CPP_FILES
  @PLUGIN_SOURCES@
)

SET(INTERNAL_CPP_FILES
  @PLUGIN_INTERNAL_SOURCES@
)

SET(UI_FILES
  @PLUGIN_UI_FILES@
)

SET(MOC_H_FILES
  @PLUGIN_MOC_H_FILES@
)

SET(RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
)

SET(RES_FILES
# uncomment the following line if you want to use Qt resources
#  @PLUGIN_RES_FILES@
)

@SET_MANIFEST_SRC@

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

