# fix to build python on i686 and i386 with the python cmake build system, the x86 path must be set as default
set(path "cmake/extensions/CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "set(_libffi_system_dir \${CMAKE_SYSTEM_PROCESSOR})" "set(_libffi_system_dir \"x86\")" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
