SET(PLUGIN_DEPENDS_ON
  org.opencherry.osgi
  #org.opencherry.core.runtime
  org.opencherry.ui
  org.opencherry.ui.qt
  
  #org.mitk.core.services
  #org.mitk.gui.common
  #org.mitk.gui.qt.application
  org.mitk.gui.qt.common
)

INCLUDE_DIRECTORIES(${Q${KITNAME}_INCLUDE_DIRS})

CREATE_QT_PLUGIN()
