#-----------------------------------------------------------------------------
# SOFA
#-----------------------------------------------------------------------------

if(MITK_USE_SOFA)
  # Sanity checks
  if(DEFINED SOFA_DIR AND NOT EXISTS ${SOFA_DIR})
    message(FATAL_ERROR "SOFA_DIR variable is defined but corresponds to non-existing directory")
  endif()

  function(write_downloadfile_script script_filename remote local md5)
    file(WRITE ${script_filename}
"message(STATUS \"downloading...
     src='${remote}'
     dst='${local}'\")

file(DOWNLOAD
  \"${remote}\"
  \"${local}\"
  SHOW_PROGRESS
  EXPECTED_MD5 ${md5}
  STATUS status
  LOG log)

list(GET status 0 status_code)
list(GET status 1 status_string)

if(NOT status_code EQUAL 0)
  message(FATAL_ERROR \"error: downloading '${remote}' failed
     status_code: \${status_code}
     status_string: \${status_string}
     log: \${log}\")
endif()

message(STATUS \"downloading... done\")")
  endfunction()

  function(write_extractfile_script script_filename filename directory)
    set(args "")

    if(filename MATCHES "(\\.|=)(bz2|tar\\.gz|tgz|zip)$")
      set(args "xfz")
    elseif(filename MATCHES "(\\.|=)tar$")
      set(args "xf")
    else()
      message(SEND_ERROR "error: do not know how to extract '${filename}' -- known types are .bz2, .tar, .tar.gz, .tgz, and .zip")
      return()
    endif()

    file(WRITE ${script_filename}
"get_filename_component(filename \"${filename}\" ABSOLUTE)
get_filename_component(directory \"${directory}\" ABSOLUTE)

message(STATUS \"extracting...
     src='\${filename}'
     dst='\${directory}'\")

if(NOT EXISTS \"\${filename}\")
  message(FATAL_ERROR \"error: file to extract does not exists: '\${filename}'\")
endif()

set(i 0)
while(EXISTS \"\${directory}/../tmp-\${i}\")
  math(EXPR i \"\${i} + 1\")
endwhile()
set(ut_dir \"\${directory}/../tmp-\${i}\")
file(MAKE_DIRECTORY \"\${ut_dir}\")

message(STATUS \"extracting... [tar ${args}]\")
execute_process(COMMAND \${CMAKE_COMMAND} -E tar ${args} \${filename} WORKING_DIRECTORY \${ut_dir} RESULT_VARIABLE rv)

if(NOT rv EQUAL 0)
  message(STATUS \"extracting... [error clean up]\")
  file(REMOVE_RECURSE \"\${ut_dir}\")
  message(FATAL_ERROR \"error: extract of '\${filename}' failed\")
endif()

message(STATUS \"extracting... [copy]\")
file(GLOB contents \"\${ut_dir}/*\")
file(COPY \${contents} DESTINATION \${directory})

message(STATUS \"extracting... [clean up]\")
file(REMOVE_RECURSE \"\${ut_dir}\")

message(STATUS \"extracting... done\")")
  endfunction()

  set(proj SOFA)
  set(proj_DEPENDENCIES)
  set(SOFA_DEPENDS ${proj})

  set(stamp_dir "${CMAKE_BINARY_DIR}/${proj}-cmake/src/${proj}-stamp")
  set(download_dir "${CMAKE_BINARY_DIR}/${proj}-cmake/src")
  set(source_dir "${CMAKE_BINARY_DIR}/${proj}-src")

  set(url_base "http://dl.dropbox.com/u/5822501/")
  set(file "sofa-framework-1.0-rc1.zip")
  set(md5 "e0f57db0d4c8178afba55c31cf7b6bdc")

  write_downloadfile_script(
    "${stamp_dir}/download-framework.cmake"
    "${url_base}${file}"
    "${download_dir}/${file}"
    "${md5}"
  )

  write_extractfile_script(
    "${stamp_dir}/extract-framework.cmake"
    "${download_dir}/${file}"
    "${source_dir}"
  )

  set(download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/download-framework.cmake)
  list(APPEND download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/extract-framework.cmake)

  set(file "sofa-modules-1.0-rc1.zip")
  set(md5 "27c9d35ea3fd4bbc753ead62f522c6e7")

  write_downloadfile_script(
    "${stamp_dir}/download-modules.cmake"
    "${url_base}${file}"
    "${download_dir}/${file}"
    "${md5}"
  )

  write_extractfile_script(
    "${stamp_dir}/extract-modules.cmake"
    "${download_dir}/${file}"
    "${source_dir}"
  )

  list(APPEND download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/download-modules.cmake)
  list(APPEND download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/extract-modules.cmake)

  if(MSVC OR APPLE)
    if(MSVC90)
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(file "sofa-dependencies-win64-msvc-2008-1.0-rc1.zip")
        set(md5 "11bc875da23e2f4e8f8094bf136d19ea")
      else()
        set(file "sofa-dependencies-win32-msvc-2008-1.0-rc1.zip")
        set(md5 "de1fedeb6468e1938857ceb07abc26fb")
      endif()
    elseif(MSVC10)
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(file "dependencies-win64-msvc-2010-1.0-rc1.zip")
        set(md5 "a6901ebe20752c5bd6a00d43df6f150e")
      else()
        set(file "dependencies-win32-msvc-2010-1.0-rc1.zip")
        set(md5 "c720994e5dedc186176b0bb711a0d5d3")
      endif()
    elseif(APPLE)
      set(file "sofa-dependencies-mac-1.0-rc1.zip")
      set(md5 "e93402fbf596693880007921985ddaea")
    endif()

    write_downloadfile_script(
      "${stamp_dir}/download-dependencies.cmake"
      "${url_base}${file}"
      "${download_dir}/${file}"
      "${md5}"
    )

    write_extractfile_script(
      "${stamp_dir}/extract-dependencies.cmake"
      "${download_dir}/${file}"
      "${source_dir}"
    )

    list(APPEND download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/download-dependencies.cmake)
    list(APPEND download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/extract-dependencies.cmake)
  endif()

  set(file "sofa-cmake-1.0-rc1.zip")
  set(md5 "10b35b4101ac655abaa51547f7356a84")

  write_downloadfile_script(
    "${stamp_dir}/download-cmake.cmake"
    "${url_base}${file}"
    "${download_dir}/${file}"
    "${md5}"
  )

  write_extractfile_script(
    "${stamp_dir}/extract-cmake.cmake"
    "${download_dir}/${file}"
    "${source_dir}"
  )

  list(APPEND download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/download-cmake.cmake)
  list(APPEND download_cmd ${CMAKE_COMMAND} -P ${stamp_dir}/extract-cmake.cmake)

  if(NOT DEFINED SOFA_DIR)
    ExternalProject_Add(${proj}
      SOURCE_DIR ${source_dir}
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      DOWNLOAD_COMMAND "${download_cmd}"
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(SOFA_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj}_DEPENDENCIES}")
  endif()
endif()
