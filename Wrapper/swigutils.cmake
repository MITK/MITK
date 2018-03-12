macro(setup_swig)
    if (NOT SWIG_DIR)
        set(SWIG_DIR $ENV{SWIG_DIR} )
    endif()
    if (NOT SWIG_EXECUTABLE)
        set(SWIG_EXECUTABLE $ENV{SWIG_EXECUTABLE} )
    endif()
    find_package(SWIG REQUIRED)
    INCLUDE(${SWIG_USE_FILE})
    message(STATUS "SWIG_DIR  = ${SWIG_DIR}" )
    message(STATUS "SWIG_EXECUTABLE  = ${SWIG_EXECUTABLE}" )
endmacro()

macro(setup_python)
    # CMake's python-detection is crazy broken, so we use python-config on *nix.
    # Sadly on Windows python-config isn't available, so do our best with CMake.
    #
    # see https://cmake.org/Bug/view.php?id=14809
    #
    if (WIN32)
        find_package(PythonInterp)
        find_package(PythonLibs)

        set(PYTHON_CFLAGS "-I${PYTHON_INCLUDE_PATH}")
        set(PYTHON_LDFLAGS "${PYTHON_LIBRARIES}")
    else()
        if (ARCH STREQUAL "arm")
            find_program(PYTHON_CONFIG_EXECUTABLE arm-linux-gnueabihf-python-config)
        else()
            find_program(PYTHON_CONFIG_EXECUTABLE python-config)
        endif()
        if (PYTHON_CONFIG_EXECUTABLE)
            execute_process(COMMAND ${PYTHON_CONFIG_EXECUTABLE} --cflags OUTPUT_VARIABLE PYTHON_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
            execute_process(COMMAND ${PYTHON_CONFIG_EXECUTABLE} --ldflags OUTPUT_VARIABLE PYTHON_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (NOT PYTHON_EXECUTABLE)
                execute_process(COMMAND ${PYTHON_CONFIG_EXECUTABLE} --exec-prefix OUTPUT_VARIABLE PYTHON_EXECUTABLE OUTPUT_STRIP_TRAILING_WHITESPACE)
                set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE}/bin/python)
            endif()
        endif()
    endif()

    if (PYTHON_EXECUTABLE)
        # get the python version
        execute_process(COMMAND ${PYTHON_EXECUTABLE} --version ERROR_VARIABLE PYTHON_VERSION)
        # message("rv='${PYTHON_VERSION}'")
        string(STRIP ${PYTHON_VERSION} PYTHON_VERSION)
        string(REPLACE " " ";" PYTHON_VERSION ${PYTHON_VERSION})
        list(GET PYTHON_VERSION 1 PYTHON_VERSION)

        message(STATUS "PYTHON_EXECUTABLE= ${PYTHON_EXECUTABLE}")
        message(STATUS "PYTHON_VERSION= ${PYTHON_VERSION}")
        message(STATUS "PYTHON_CFLAGS= ${PYTHON_CFLAGS}")
        message(STATUS "PYTHON_LDFLAGS= ${PYTHON_LDFLAGS}")

        set(PYTHON 1)
    endif()
endmacro()

# build-time file replacement
set(FILE_REPLACE_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/replace.cmake)
file(WRITE  ${FILE_REPLACE_SCRIPT} "file(READ \${FROM} file_contents) \n")
file(APPEND ${FILE_REPLACE_SCRIPT} "string(REPLACE \${MATCH_STRING} \${REPLACE_STRING} file_contents \${file_contents}) \n")
file(APPEND ${FILE_REPLACE_SCRIPT} "file(WRITE \${TO} \${file_contents}) \n")
