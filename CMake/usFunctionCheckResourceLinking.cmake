#! \ingroup MicroServicesCMake
#! \brief Check that resources can be linked into binaries.
#!
#! Sets the cache-internal variable US_RESOURCE_SOURCE_SUFFIX, the extension
#! of the resource source that usFunctionGetResourceSource and
#! usFunctionEmbedResources generate: an object file assembled from the ZIP
#! archive on Linux and macOS, a resource script on Windows.
#!
#! Resources are always linked into a regular section of the binary, or into a
#! PE resource on Windows, where they survive strip and code signing and the
#! module reads them from memory. A toolchain that cannot do this is not
#! supported, so the check fails the configuration instead of building modules
#! without resources.
#!
#! The check runs on every configure, so a build tree follows a changed
#! toolchain.
#!
#! \sa usFunctionEmbedResources
function(usFunctionCheckResourceLinking)
  set(_suffix "")
  if(APPLE)
    set(_result )
    usFunctionCheckCompilerFlags("-Wl,-sectcreate,__TEXT,us_resources,CMakeLists.txt" _result)
    if(_result)
      set(_suffix .o)
    endif()
  elseif(WIN32)
    if(CMAKE_RC_COMPILER)
      set(_suffix .rc)
    endif()
  elseif(UNIX AND CMAKE_OBJCOPY)
    set(_test_object "${CMAKE_CURRENT_BINARY_DIR}/us_resource_link.o")
    execute_process(
      COMMAND ${CMAKE_LINKER} -r -b binary -o "${_test_object}" "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"
      RESULT_VARIABLE _result
      OUTPUT_QUIET
      ERROR_QUIET
    )
    file(REMOVE "${_test_object}")
    if(_result EQUAL 0)
      set(_suffix .o)
    endif()
  endif()

  if(NOT _suffix)
    message(FATAL_ERROR "CppMicroServices resources cannot be linked with this toolchain: "
      "it requires the linker to embed a binary file (-sectcreate on macOS, "
      "ld -r -b binary plus objcopy on Linux) or a resource compiler on Windows.")
  endif()

  message(STATUS "Checking for CppMicroServices resource linking capability...yes")
  set(US_RESOURCE_SOURCE_SUFFIX ${_suffix} CACHE INTERNAL "CppMicroServices resource source suffix" FORCE)
endfunction()
