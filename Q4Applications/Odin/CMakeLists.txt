ADD_EXECUTABLE(Odin Odin.cpp)

TARGET_LINK_LIBRARIES(Odin org_opencherry_osgi)

CONFIGURE_FILE(config.ini
               ${EXECUTABLE_OUTPUT_PATH}/config.ini)
