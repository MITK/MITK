SET(SRC_CPP_FILES
  @PLUGIN_SOURCES@
)

SET(INTERNAL_CPP_FILES
  @PLUGIN_INTERNAL_SOURCES@
)

@SET_MANIFEST_SRC@

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

