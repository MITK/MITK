
MACRO_CREATE_PLUGIN()

TARGET_LINK_LIBRARIES(${PLUGIN_TARGET} optimized PocoUtil debug PocoUtild optimized PocoXML debug PocoXMLd optimized mbilog)
