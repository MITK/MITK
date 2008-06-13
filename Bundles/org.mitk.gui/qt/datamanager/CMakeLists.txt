SET(PLUGIN_DEPENDS_ON
  org.opencherry.osgi
  org.opencherry.ui
  org.opencherry.ui.qt
  
  org.mitk.gui.qt.application
)

INCLUDE_DIRECTORIES(${Q${KITNAME}_INCLUDE_DIRS})

CREATE_QT_PLUGIN()
