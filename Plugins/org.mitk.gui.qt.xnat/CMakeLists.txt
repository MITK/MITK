project(org_mitk_gui_qt_xnat)

find_package(OpenSSL)

if( NOT OPENSSL_FOUND AND NOT SSL_EAY_RELEASE AND NOT LIB_EAY_RELEASE )
  message( "Could not find OpenSSL, XNAT will not work for HTTPS connections. Please set the SSL_EAY_RELEASE and LIB_EAY_RELEASE CMake variables to the respective libraries." )
endif()

if( WIN32 AND EXISTS ${SSL_EAY_RELEASE} AND EXISTS ${LIB_EAY_RELEASE} )
  configure_file( ${SSL_EAY_RELEASE} ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
  configure_file( ${LIB_EAY_RELEASE} ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
  configure_file( ${SSL_EAY_RELEASE} ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
  configure_file( ${LIB_EAY_RELEASE} ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
  MITK_INSTALL( FILES ${SSL_EAY_RELEASE})
  MITK_INSTALL( FILES ${LIB_EAY_RELEASE})
endif()

mitk_create_plugin(
  PACKAGE_DEPENDS Poco|Zip
  MODULE_DEPENDS MitkXNAT
  EXPORT_DIRECTIVE XNAT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)
