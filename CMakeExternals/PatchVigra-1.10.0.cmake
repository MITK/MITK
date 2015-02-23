# Called by Vigra.cmake (ExternalProject_Add) as a patch for Vigra.

# read whole file
file(STRINGS CMakeLists.txt sourceCode NEWLINE_CONSUME)

# Changing the way hdf5 is found. The Vigra FindHDF5.cmake file makes
# too many assumptions about the build structure of HDF5. Instead, we
# just use the hdf5-config.cmake file from our HDF5 install tree.
string(REPLACE "VIGRA_FIND_PACKAGE(HDF5)" "FIND_PACKAGE(HDF5 PATHS \"${HDF5_DIR}\" PATH_SUFFIXES hdf5 NO_DEFAULT_PATH NO_MODULE)\nIF(NOT HDF5_FOUND)\n  VIGRA_FIND_PACKAGE(HDF5)\nENDIF()" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} CMakeLists.txt @ONLY)

# patch the VigraConfig.cmake.in file
file(STRINGS config/VigraConfig.cmake.in sourceCode NEWLINE_CONSUME)
string(REPLACE "include(\${SELF_DIR}/vigra-targets.cmake)" "if(NOT TARGET vigraimpex)\n  include(\${SELF_DIR}/vigra-targets.cmake)\nendif()" sourceCode ${sourceCode})
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} config/VigraConfig.cmake.in @ONLY)
