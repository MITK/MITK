SET(PLUGIN_REQUIRED_LIBS 
  org_opencherry_osgi
  ${${KITNAME}_CORE_LIBRARIES}
)

CREATE_PLUGIN(org.mitk.core.services)
