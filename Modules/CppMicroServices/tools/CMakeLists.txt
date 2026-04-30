add_executable(usResourceCompiler usResourceCompiler.c)

target_include_directories(usResourceCompiler PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_compile_definitions(usResourceCompiler PRIVATE
  US_MINIZ_NO_ARCHIVE_READING_API
  US_MINIZ_NO_ZLIB_COMPATIBLE_NAMES)

# Vendored miniz relies on C99 non-constant aggregate initializers. MSVC
# treats those as a MSVC-C89 extension (C4204) under /TC default; request
# C11 so the construct is standard and the per-function pragma in
# us_miniz.c can stay dropped.
set_target_properties(usResourceCompiler PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED ON)

if(WIN32)
  target_link_libraries(usResourceCompiler PRIVATE Shlwapi)
endif()

set_property(TARGET usResourceCompiler PROPERTY
  FOLDER "${MITK_ROOT_FOLDER}/Modules/CppMicroServices")
