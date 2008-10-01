SET(PLUGIN_DEPENDS_ON
  org.opencherry.osgi
  org.opencherry.core.runtime
  org.opencherry.ui  
)

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/internal)

CREATE_QT_PLUGIN()
