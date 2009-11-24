set(LIBNAME "PocoUtil")
#set(LIBNAMED "${LIBNAME}d")

set( BASE_SRCS
src/AbstractConfiguration.cpp
src/Application.cpp
src/ConfigurationMapper.cpp
src/ConfigurationView.cpp
src/FilesystemConfiguration.cpp
src/HelpFormatter.cpp
src/IniFileConfiguration.cpp
src/IntValidator.cpp
src/LayeredConfiguration.cpp
src/LoggingConfigurator.cpp
src/LoggingSubsystem.cpp
src/MapConfiguration.cpp
src/Option.cpp
src/OptionCallback.cpp
src/OptionException.cpp
src/OptionProcessor.cpp
src/OptionSet.cpp
src/PropertyFileConfiguration.cpp
src/RegExpValidator.cpp
src/ServerApplication.cpp
src/Subsystem.cpp
src/SystemConfiguration.cpp
src/Validator.cpp
src/XMLConfiguration.cpp
src/Timer.cpp
src/TimerTask.cpp
)

set( WIN_SRCS
src/WinRegistryConfiguration.cpp
src/WinRegistryKey.cpp
src/WinService.cpp
)

if(CMAKE_SYSTEM MATCHES "Windows")
 set(SRCS ${BASE_SRCS} ${WIN_SRCS})
 add_definitions( -DUtil_EXPORTS )
else (CMAKE_SYSTEM MATCHES "Windows")
 set(SRCS ${BASE_SRCS})
endif(CMAKE_SYSTEM MATCHES "Windows")

add_library( ${LIBNAME} ${LIB_MODE} ${SRCS} )
set_target_properties( ${LIBNAME} PROPERTIES
  #COMPILE_FLAGS ${RELEASE_CXX_FLAGS}
	VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}
	SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
target_link_libraries( ${LIBNAME} PocoXML PocoFoundation)

#add_library( ${LIBNAMED} ${LIB_MODE} ${SRCS} )
#set_target_properties( ${LIBNAMED}  
#	PROPERTIES COMPILE_FLAGS "${DEBUG_CXX_FLAGS}" 
#       DEFINE_SYMBOL Util_EXPORTS
#	VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}
#	SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
#target_link_libraries( ${LIBNAMED} PocoXMLd PocoFoundationd )

# This is needed for a "SDK" install
#install(
#    DIRECTORY include/Poco
#    DESTINATION include
#    PATTERN ".svn" EXCLUDE
#    )
    
if (WIN32)
  install(
    TARGETS ${LIBNAME} #${LIBNAMED}
    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib
    )
else ()
  install(TARGETS ${LIBNAME}
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
  )
endif ()
