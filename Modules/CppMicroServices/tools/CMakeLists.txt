add_executable(usResourceCompiler usResourceCompiler.c)

target_include_directories(usResourceCompiler PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_compile_definitions(usResourceCompiler PRIVATE
  US_MINIZ_NO_ARCHIVE_READING_API
  US_MINIZ_NO_ZLIB_COMPATIBLE_NAMES)

if(WIN32)
  target_link_libraries(usResourceCompiler PRIVATE Shlwapi)
endif()

set_property(TARGET usResourceCompiler PROPERTY
  FOLDER "${MITK_ROOT_FOLDER}/Modules/CppMicroServices")
