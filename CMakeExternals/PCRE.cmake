
if(MITK_USE_PCRE)
  # Sanity checks
  if(DEFINED PCRE_DIR AND NOT EXISTS ${PCRE_DIR})
    message(FATAL_ERROR "PCRE_DIR variable is defined but corresponds to non-existing directory")
  endif()

  # Set dependency list
  set(proj PCRE)
  set(${proj}_DEPENDENCIES "")

  if(NOT PCRE_DIR)
    #
    #  PCRE (Perl Compatible Regular Expressions)
    #

    set(PCRE_TARGET_VERSION 8.35)
    set(PCRE_DOWNLOAD_SOURCE_HASH "ed58bcbe54d3b1d59e9f5415ef45ce1c")

    set(pcre_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
    set(pcre_source_dir ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src)
    set(pcre_install_dir ${CMAKE_CURRENT_BINARY_DIR}/PCRE-install)

    configure_file(
      ${MITK_SOURCE_DIR}/CMakeExternals/pcre_configure_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/pcre_configure_step.cmake
      @ONLY)

    set ( pcre_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/pcre_configure_step.cmake )

    ExternalProject_add(${proj}
      URL http://midas3.kitware.com/midas/api/rest?method=midas.bitstream.download&checksum=${PCRE_DOWNLOAD_SOURCE_HASH}&name=pcre-${PCRE_TARGET_VERSION}.tar.gz
      URL_MD5 "${PCRE_DOWNLOAD_SOURCE_HASH}"
      SOURCE_DIR ${pcre_source_dir}
      BINARY_DIR ${pcre_binary_dir}
      INSTALL_DIR ${pcre_install_dir}
      PREFIX ${proj}-cmake
      CONFIGURE_COMMAND ${pcre_CONFIGURE_COMMAND}
      DEPENDS "${${proj}_DEPENDENCIES}"
      )
  else()
    mitkMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
  endif()
endif()
