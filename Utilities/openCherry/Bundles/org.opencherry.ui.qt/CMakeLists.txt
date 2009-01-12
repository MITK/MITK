SET(PLUGIN_DEPENDS_ON
  org.opencherry.osgi
  org.opencherry.core.runtime
  org.opencherry.ui
)

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/internal)
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/internal/defaultpresentation)
CREATE_QT_PLUGIN()
