SET(PLUGIN_DEPENDS_ON
  org.opencherry.osgi
  org.opencherry.core.runtime
  org.opencherry.ui
  org.opencherry.ui.qt
  org.mitk.core.services
)

INCLUDE_DIRECTORIES(${Q${KITNAME}_INCLUDE_DIRS})

CREATE_QT_PLUGIN()

TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} Qmitk)
