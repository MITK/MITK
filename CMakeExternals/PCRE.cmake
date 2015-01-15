#--------------------------------------------------------------------------
#  PCRE (Perl Compatible Regular Expressions)
#--------------------------------------------------------------------------
if(MITK_USE_PCRE)
  if(DEFINED PCRE_DIR AND NOT EXISTS ${PCRE_DIR})
      message(FATAL_ERROR "PCRE_DIR variable is defined but corresponds to non-existing directory")
   endif()
  if(NOT PCRE_DIR)

    set(proj PCRE)
    set(${proj}_DEPENDENCIES "")
    set(${proj}_DEPENDS ${proj})

    ExternalProject_add(${proj}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/pcre-8.35.tar.gz
      URL_MD5 "ed58bcbe54d3b1d59e9f5415ef45ce1c"
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src
      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build
      INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
      PREFIX ${proj}-cmake
      CMAKE_ARGS
       ${ep_common_args}
       "-DCMAKE_C_FLAGS:STRING=${CMAKE_CXX_FLAGS} -fPIC"
       -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       -DBUILD_SHARED_LIBS:BOOL=OFF
       -DPCRE_BUILD_PCREGREP:BOOL=OFF
       -DPCRE_BUILD_TESTS:BOOL=OFF
       -DPCRE_SUPPORT_JIT:BOOL=ON
      DEPENDS "${${proj}_DEPENDENCIES}"
      )

    set(PCRE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)

  else()
    mitkMacroEmptyExternalProject(${proj} "${${proj}_DEPENDENCIES}")
  endif()
endif()
