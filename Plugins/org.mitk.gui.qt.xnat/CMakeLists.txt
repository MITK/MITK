project(org_mitk_gui_qt_xnat)

if(WIN32)
  set(MITK_OPENSSL_SSL_DLL "" CACHE FILEPATH "")
  set(MITK_OPENSSL_CRYPTO_DLL "" CACHE FILEPATH "")

  if(MITK_OPENSSL_SSL_DLL AND EXISTS "${MITK_OPENSSL_SSL_DLL}" AND MITK_OPENSSL_CRYPTO_DLL AND EXISTS "${MITK_OPENSSL_CRYPTO_DLL}")
    foreach(config_type ${CMAKE_CONFIGURATION_TYPES})
      execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${MITK_BINARY_DIR}/bin/${config_type}")
      configure_file("${MITK_OPENSSL_SSL_DLL}" "${MITK_BINARY_DIR}/bin/${config_type}/" COPYONLY)
      configure_file("${MITK_OPENSSL_CRYPTO_DLL}" "${MITK_BINARY_DIR}/bin/${config_type}/" COPYONLY)
    endforeach()

    MITK_INSTALL(FILES
      "${MITK_OPENSSL_SSL_DLL}"
      "${MITK_OPENSSL_CRYPTO_DLL}"
    )
  endif()
endif()

mitk_create_plugin(
  PACKAGE_DEPENDS Poco|Zip
  MODULE_DEPENDS MitkXNAT
  EXPORT_DIRECTIVE XNAT_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
)
