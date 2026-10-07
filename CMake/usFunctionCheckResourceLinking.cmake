#! \ingroup MicroServicesCMake
#! \brief Detect resource linking support and choose the default resource mode.
#!
#! Sets the cache-internal variables US_RESOURCE_LINKING_AVAILABLE,
#! US_DEFAULT_RESOURCE_MODE (LINK or APPEND), US_RESOURCE_SOURCE_SUFFIX,
#! US_RESOURCE_SOURCE_SUFFIX_LINK and US_RESOURCE_SOURCE_SUFFIX_APPEND, which
#! usFunctionGetResourceSource and usFunctionEmbedResources rely on.
#!
#! The detection runs on every configure. The mode and the source suffix must
#! always agree, and a build tree that cached them once would never follow a
#! changed default: it would keep the old suffix in the source lists while the
#! embed step produces the other one, which silently drops every resource.
#!
#! LINK is the default wherever it is available, except on Windows. Appended
#! resources sit behind the end of the ELF or Mach-O structure, so strip
#! discards them and macOS code signing rejects them; linked resources live in
#! a regular section. Windows keeps APPEND until its resource-compiler path has
#! been verified.
#!
#! \sa usFunctionEmbedResources
function(usFunctionCheckResourceLinking)
  set(_suffix )
  set(_linking_available 0)
  if(APPLE)
    set(_result )
    usFunctionCheckCompilerFlags("-Wl,-sectcreate,__TEXT,us_resources,CMakeLists.txt" _result)
    if(_result)
      set(_linking_available 1)
    endif()
    set(_suffix .o)
  elseif(WIN32 AND CMAKE_RC_COMPILER)
    set(_linking_available 1)
    set(_suffix .rc)
  elseif(UNIX AND CMAKE_OBJCOPY)
    set(_test_object "${CMAKE_CURRENT_BINARY_DIR}/us_resource_link.o")
    execute_process(
      COMMAND ${CMAKE_LINKER} -r -b binary -o "${_test_object}" "${CMAKE_COMMAND}"
      RESULT_VARIABLE _result
      OUTPUT_QUIET
      ERROR_QUIET
    )
    file(REMOVE "${_test_object}")
    if(_result EQUAL 0)
      set(_linking_available 1)
    endif()
    set(_suffix .o)
  endif()

  set(US_RESOURCE_SOURCE_SUFFIX_LINK ${_suffix} CACHE INTERNAL "CppMicroServices resource source suffix (link)" FORCE)
  set(US_RESOURCE_SOURCE_SUFFIX_APPEND ".cpp" CACHE INTERNAL "CppMicroServices resource source suffix (append)" FORCE)

  set(_success "no")
  set(_default_mode "APPEND")
  if(_linking_available)
    set(_success "yes")
    if(NOT WIN32)
      set(_default_mode "LINK")
    endif()
  endif()

  message(STATUS "Checking for CppMicroServices resource linking capability...${_success} (default mode: ${_default_mode})")

  set(US_RESOURCE_LINKING_AVAILABLE ${_linking_available} CACHE INTERNAL "CppMicroServices resource linking" FORCE)
  set(US_DEFAULT_RESOURCE_MODE ${_default_mode} CACHE INTERNAL "CppMicroServices default resource mode" FORCE)
  if(_default_mode STREQUAL "LINK")
    set(US_RESOURCE_SOURCE_SUFFIX ${US_RESOURCE_SOURCE_SUFFIX_LINK} CACHE INTERNAL "CppMicroServices resource source suffix" FORCE)
  else()
    set(US_RESOURCE_SOURCE_SUFFIX ${US_RESOURCE_SOURCE_SUFFIX_APPEND} CACHE INTERNAL "CppMicroServices resource source suffix" FORCE)
  endif()
endfunction()
