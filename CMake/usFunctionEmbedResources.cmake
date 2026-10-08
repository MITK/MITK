#! \ingroup MicroServicesCMake
#! \brief Embed resources in a library or executable.
#!
#! This CMake function uses an external command line program to generate a ZIP archive
#! containing data from external resources such as text files or images or other ZIP
#! archives. The created archive is linked into the target file: into a section of its
#! own on Linux and macOS, as a PE resource on Windows. There it survives tools that
#! rewrite the binary, such as strip or codesign, and the module reads it from memory
#! (see usModuleInitialization.h).
#!
#! \note To set-up correct file dependencies from your module target to your resource
#!       files, you have to add a special source file to the source list of the target.
#!       The source file name can be retrieved by using usFunctionGetResourceSource.
#!       This ensures that changed resource files will automatically be re-added to the
#!       module.
#!
#! Example usage:
#! \code{.cmake}
#! set(module_srcs )
#! usFunctionEmbedResources(TARGET mylib
#!                        MODULE_NAME org_me_mylib
#!                        FILES config.properties logo.png
#!                       )
#! \endcode
#!
#! \param TARGET (required) The target to which the resource files are added.
#! \param MODULE_NAME (required/optional) The module name of the target, as specified in
#!        the \c US_MODULE_NAME pre-processor definition of that target. This parameter
#!        is optional if a target property with the name US_MODULE_NAME exists, containing
#!        the required module name.
#!
#! For the WORKING_DIRECTORY, COMPRESSION_LEVEL, FILES, ZIP_ARCHIVES parameters see the
#! documentation of the usFunctionAddResources macro which is called with these parameters if set.
#!
#! \sa usFunctionAddResources
#! \sa usFunctionGetResourceSource
#! \sa \ref MicroServices_Resources
#!
set(_us_embed_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")

function(usFunctionEmbedResources)

  cmake_parse_arguments(US_RESOURCE "" "TARGET;MODULE_NAME;WORKING_DIRECTORY;COMPRESSION_LEVEL" "FILES;ZIP_ARCHIVES" ${ARGN})

  if(NOT US_RESOURCE_TARGET)
    message(SEND_ERROR "TARGET argument not specified.")
  endif()

  if(US_RESOURCE_FILES OR US_RESOURCE_ZIP_ARCHIVES)
    usFunctionAddResources(TARGET ${US_RESOURCE_TARGET}
      MODULE_NAME ${US_RESOURCE_MODULE_NAME}
      WORKING_DIRECTORY ${US_RESOURCE_WORKING_DIRECTORY}
      COMPRESSION_LEVEL ${US_RESOURCE_COMPRESSION_LEVEL}
      FILES ${US_RESOURCE_FILES}
      ZIP_ARCHIVES ${US_RESOURCE_ZIP_ARCHIVES}
    )
  endif()

  get_target_property(_res_zips ${US_RESOURCE_TARGET} _us_resource_zips)
  if(NOT _res_zips)
    return()
  endif()

  usFunctionGetResourceSource(TARGET ${US_RESOURCE_TARGET} OUT _source_output)

  set(resource_compiler usResourceCompiler)

  set(_zip_archive )
  get_target_property(_counter ${US_RESOURCE_TARGET} _us_resource_counter)
  if(_counter EQUAL 0)
    set(_zip_archive ${_res_zips})
  else()
    set(_zip_archive ${CMAKE_CURRENT_BINARY_DIR}/us_${US_RESOURCE_TARGET}/res.zip)
    add_custom_command(
      OUTPUT ${_zip_archive}
      COMMAND ${resource_compiler} ${_zip_archive} dummy -m ${_res_zips}
      DEPENDS ${_res_zips} ${resource_compiler}
      COMMENT "Creating resources zip file for ${US_RESOURCE_TARGET}"
      VERBATIM
     )
  endif()
  get_filename_component(_zip_archive_name ${_zip_archive} NAME)
  get_filename_component(_zip_archive_path ${_zip_archive} PATH)

  if(APPLE)
    add_custom_command(
      OUTPUT ${_source_output}
      COMMAND ${CMAKE_CXX_COMPILER} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -c ${_us_embed_cmake_dir}/usCMakeResourceDependencies.cpp -o stub.o
      COMMAND ${CMAKE_LINKER} -r -sectcreate __TEXT us_resources ${_zip_archive_name} stub.o -o ${_source_output}
      DEPENDS ${_zip_archive}
      WORKING_DIRECTORY ${_zip_archive_path}
      COMMENT "Linking resources zip file for ${US_RESOURCE_TARGET}"
      VERBATIM
     )
    set_source_files_properties(${_source_output} PROPERTIES EXTERNAL_OBJECT 1 GENERATED 1)
  elseif(WIN32)
    set(US_RESOURCE_ARCHIVE ${_zip_archive})
    configure_file(${_us_embed_cmake_dir}/us_resources.rc.in ${_source_output})
    add_custom_command(
      OUTPUT ${_source_output}
      COMMAND ${CMAKE_COMMAND} -E touch ${_source_output}
      DEPENDS ${_zip_archive}
      WORKING_DIRECTORY ${_zip_archive_path}
      COMMENT "Linking resources zip file for ${US_RESOURCE_TARGET}"
      VERBATIM
     )
  else()
    # `ld -r -b binary` puts the archive into .data. Move it into a read-only
    # section of its own, whose bounds the linker provides for each binary as
    # __start_us_resources and __stop_us_resources (see
    # usModuleInitialization.h). The generic _binary_* symbols named after
    # the input file would otherwise be exported by every module.
    string(MAKE_C_IDENTIFIER "${_zip_archive_name}" _zip_symbol)
    add_custom_command(
      OUTPUT ${_source_output}
      COMMAND ${CMAKE_LINKER} -r -b binary -o ${_source_output} ${_zip_archive_name}
      COMMAND ${CMAKE_OBJCOPY} --rename-section .data=us_resources,alloc,load,readonly,data,contents
        --strip-symbol _binary_${_zip_symbol}_start
        --strip-symbol _binary_${_zip_symbol}_end
        --strip-symbol _binary_${_zip_symbol}_size
        ${_source_output} ${_source_output}
      DEPENDS ${_zip_archive}
      WORKING_DIRECTORY ${_zip_archive_path}
      COMMENT "Linking resources zip file for ${US_RESOURCE_TARGET}"
      VERBATIM
     )
    set_source_files_properties(${_source_output} PROPERTIES EXTERNAL_OBJECT 1 GENERATED 1)
    # The resource object above is assembled from a raw ZIP blob via `ld -r -b
    # binary`, so it has no `.note.GNU-stack` section. Older GNU ld versions
    # take a missing note as a request for an executable stack, and glibc
    # 2.41 and newer refuse to dlopen such a module, so mark the stack
    # non-executable explicitly.
    target_link_options(${US_RESOURCE_TARGET} PRIVATE "LINKER:-z,noexecstack")
  endif()

endfunction()
