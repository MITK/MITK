#
#  Windows python 2.7.3 patches for the CMake build system
#  https://github.com/davidsansome/python-cmake-buildsystem/tree/master/cmake/patches-win32
#
set(path "Lib/distutils/msvc9compiler.py")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "self.manifest_get_embed_info(target_desc, ld_args)" "None" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)
