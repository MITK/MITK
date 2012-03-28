if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif(CMAKE_COMPILER_IS_GNUCXX)

if(MITK_USE_Python)
  # Python include directory.
  set(MITK_INCLUDE_DIRS ${MITK_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_PATH})
endif(MITK_USE_Python)

# We have found CableSwig.  Use the settings.
set(CABLE_INDEX ${CableSwig_cableidx_EXE})
set(CSWIG ${CableSwig_cswig_EXE})
set(GCCXML ${CableSwig_gccxml_EXE})
set(MITK_WRAPPING_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MITK_WRAPPING_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

#set(MITK_WRAP_NEEDS_DEPEND 1)
#if(${CMAKE_MAKE_PROGRAM} MATCHES make)
  #set(MITK_WRAP_NEEDS_DEPEND 0)
#endif(${CMAKE_MAKE_PROGRAM} MATCHES make)
set(MITK_TOP ${MITK_SOURCE_DIR})
set(MITK_SWIG_DEFAULT_LIB ${CableSwig_DIR}/Swig/Lib )
set(MITK_WRAP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MITK_WRAP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CSWIG_EXTRA_LINKFLAGS )
if(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  set(CSWIG_EXTRA_LINKFLAGS "/IGNORE:4049")
endif(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")

if(CMAKE_SYSTEM MATCHES "IRIX.*")
  if(CMAKE_CXX_COMPILER MATCHES "CC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -woff 1552")
  endif(CMAKE_CXX_COMPILER MATCHES "CC")
endif(CMAKE_SYSTEM MATCHES "IRIX.*")

if(CMAKE_COMPILER_IS_GNUCXX)
  string(REGEX REPLACE "-Wcast-qual" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(IGNORE_WARNINGS -w362 -w389 -w503 -w508 -w509 -w516)
# define macros for wrapping commands
macro(GCCXML_CREATE_XML_FILE Source Bin Input Output Library)
# if the make program is not an IDE then include
# the depend file in a way that will make cmake
# re-run if it changes
   set(CABLE_SWIG_DEPEND)
   set(CABLE_SWIG_DEPEND_REGENERATE)
   if(${CMAKE_MAKE_PROGRAM} MATCHES "make")
     if(EXISTS ${Bin}/${Output}.depend)
     else(EXISTS ${Bin}/${Output}.depend)
       configure_file(
         ${MITK_SOURCE_DIR}/Wrapping/CSwig/empty.depend.in
         ${Bin}/${Output}.depend @ONLY IMMEDIATE)
     endif(EXISTS ${Bin}/${Output}.depend)
     include(${Bin}/${Output}.depend)
   else(${CMAKE_MAKE_PROGRAM} MATCHES "make")
# for IDE generators like MS dev only include the depend files
# if they exist.   This is to prevent ecessive reloading of
# workspaces after each build.   This also means
# that the depends will not be correct until cmake
# is run once after the build has completed once.
# the depend files are created in the wrap tcl/python sections
# when the .xml file is parsed.
     include(${Bin}/${Output}.depend OPTIONAL)
   endif(${CMAKE_MAKE_PROGRAM} MATCHES "make")

   if(CABLE_SWIG_DEPEND)
     # There are dependencies.  Make sure all the files are present.
     # If not, force the rule to re-run to update the dependencies.
     foreach(f ${CABLE_SWIG_DEPEND})
       if(EXISTS ${f})
       else(EXISTS ${f})
         set(CABLE_SWIG_DEPEND_REGENERATE 1)
       endif(EXISTS ${f})
     endforeach(f)
   else(CABLE_SWIG_DEPEND)
     # No dependencies, make the output depend on the dependency file
     # itself, which should cause the rule to re-run.
     set(CABLE_SWIG_DEPEND_REGENERATE 1)
   endif(CABLE_SWIG_DEPEND)
   if(CABLE_SWIG_DEPEND_REGENERATE)
     set(CABLE_SWIG_DEPEND ${Bin}/${Output}.depend)
     configure_file(
       ${MITK_SOURCE_DIR}/Wrapping/CSwig/empty.depend.in
       ${Bin}/${Output}.depend @ONLY IMMEDIATE)
   endif(CABLE_SWIG_DEPEND_REGENERATE)
  set(OutDir "Debug")

   if(WIN32)
   set(OS -DWIN32)
   endif(WIN32)

   add_custom_command(
   COMMENT "${Output} from "
   SOURCE ${Source}/${Input}
   COMMAND ${GCCXML}
   ARGS -fxml-start=_cable_
        -fxml=${Bin}/${Output} --gccxml-gcc-options ${SWIG_INC_FILE}
    ${OS}
        -DCSWIG -DCABLE_CONFIGURATION ${Source}/${Input}
   TARGET ${Library}
   OUTPUTS ${Bin}/${Output}
   DEPENDS ${GCCXML} ${CABLE_SWIG_DEPEND})

endmacro(GCCXML_CREATE_XML_FILE)


macro(CINDEX_CREATE_IDX_FILE Bin Input Output Library)
   add_custom_command(
   COMMENT "${Output} from "
   SOURCE ${Bin}/${Input}
   COMMAND ${CABLE_INDEX}
   ARGS ${Bin}/${Input} ${Bin}/${Output}
   TARGET ${Library}
   OUTPUTS ${Bin}/${Output}
   DEPENDS ${Bin}/${Input} ${CABLE_INDEX}
)
endmacro(CINDEX_CREATE_IDX_FILE)

set(MITK_USE_Python_NO_EXCEPTION_REGEX "(ContinuousIndex|Python)\\.xml$")

macro(CSWIG_CREATE_PYTHON_CXX_FILE Bin MasterIdx InputIdx InputXml OutputPythonCxx Library LibraryIndexFiles)
   set(CINDEX)
   foreach(MIDX ${MasterIdx})
     set(CINDEX ${CINDEX} -Cindex "${MIDX}")
   endforeach(MIDX)
   if("${InputXml}" MATCHES "${MITK_USE_Python_NO_EXCEPTION_REGEX}")
     set(MITK_SWG_FILE "")
   else("${InputXml}" MATCHES "${MITK_USE_Python_NO_EXCEPTION_REGEX}")
     set(MITK_SWG_FILE "-l${MITK_CSWIG_SOURCE_DIR}/mitk.swg" "-l${MITK_CSWIG_SOURCE_DIR}/mitkvtk.swg")
   endif("${InputXml}" MATCHES "${MITK_USE_Python_NO_EXCEPTION_REGEX}")
   string(REPLACE "_" "" LIB_PYTHON_FILES ${Library})

   #message(STATUS "CMAKE_CFG_INTDIR: ${CMAKE_CFG_INTDIR}")
   #message(STATUS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${LIB_PYTHON_FILES}")
   #make_directory(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${LIB_PYTHON_FILES})
   make_directory(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIB_PYTHON_FILES})
   file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIB_PYTHON_FILES}/__init__.py "from mitkCorePython import *")

   add_custom_command(
     COMMENT "${OutputPythonCxx} from "
     SOURCE ${Bin}/${InputIdx}
     COMMAND ${CSWIG}
     ARGS ${MITK_SWG_FILE}
          -I${MITK_SWIG_DEFAULT_LIB}
          -I${MITK_SWIG_DEFAULT_LIB}/python
          -noruntime
      -dirprot
      #-keyword
      ${CINDEX} ${IGNORE_WARNINGS} -depend ${Bin}/${InputXml}.depend
          -outdir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CFG_INTDIR}/${LIB_PYTHON_FILES}"
          -o ${Bin}/${OutputPythonCxx} -python -c++ ${Bin}/${InputXml}
     TARGET ${Library}
     OUTPUTS ${Bin}/${OutputPythonCxx}
     DEPENDS ${LibraryIndexFiles}  ${MITK_CSWIG_SOURCE_DIR}/mitk.swg ${MITK_CSWIG_SOURCE_DIR}/mitk.swg ${Bin}/${InputXml} ${CSWIG} )
endmacro(CSWIG_CREATE_PYTHON_CXX_FILE)

# macro to create .xml, .idx and Python.cxx files
macro(WRAP_PYTHON_SOURCES Source Bin BaseName LibraryName MasterIdx LibraryIndexFiles)
   GCCXML_CREATE_XML_FILE(${Source} ${Bin} ${BaseName}.cxx ${BaseName}.xml ${LibraryName})
   CINDEX_CREATE_IDX_FILE(${Bin} ${BaseName}.xml  ${BaseName}.idx ${LibraryName})
   CSWIG_CREATE_PYTHON_CXX_FILE(${Bin} "${MasterIdx}" ${BaseName}.idx ${BaseName}.xml
                             ${BaseName}Python.cxx ${LibraryName} "${LibraryIndexFiles}")
endmacro(WRAP_PYTHON_SOURCES)

# make sure required stuff is set
if(NOT EXECUTABLE_OUTPUT_PATH)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
endif(NOT EXECUTABLE_OUTPUT_PATH)

set(MITK_CSWIG_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(MITK_CSWIG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(MITK_LIBRARY_DIRS)
  string(REGEX REPLACE "^/" "" MITK_INSTALL_LIBRARY_DIR "${MITK_LIBRARY_DIRS}")
endif(MITK_LIBRARY_DIRS)
if(MITK_BINARY_DIR)
  string(REGEX REPLACE "^/" "" MITK_INSTALL_BIN_DIR "${MITK_BINARY_DIR}")
endif(MITK_BINARY_DIR)
set(CSWIG_MISSING_VALUES)
if(NOT CSWIG)
   set(CSWIG_MISSING_VALUES "${CSWIG_MISSING_VALUES} CSWIG ")
endif(NOT CSWIG)
if(NOT CABLE_INDEX)
   set(CSWIG_MISSING_VALUES "${CSWIG_MISSING_VALUES} CABLE_INDEX ")
endif(NOT CABLE_INDEX)
if(NOT GCCXML)
   set(CSWIG_MISSING_VALUES "${CSWIG_MISSING_VALUES} GCCXML ")
endif(NOT GCCXML)
if(CSWIG_MISSING_VALUES)
  message(SEND_ERROR "To use cswig wrapping, CSWIG, CABLE_INDEX, and GCCXML executables must be specified.  If they are all in the same directory, only specifiy one of them, and then run cmake configure again and the others should be found.\nCurrently, you are missing the following:\n ${CSWIG_MISSING_VALUES}")
endif(CSWIG_MISSING_VALUES)


if(MITK_USE_Python)
  include_directories(${PYTHON_INCLUDE_PATH} )
endif(MITK_USE_Python)

set(SWIG_INC
  ${MITK_INCLUDE_DIRS_BUILD_TREE}
  ${MITK_INCLUDE_DIRS_BUILD_TREE_CXX}
  ${MITK_INCLUDE_DIRS}
  ${MITK_TOP}/Wrapping/CSwig
  ${MITK_TOP}/Wrapping/CSwig/Core
  )
add_definitions(-DSWIG_GLOBAL)

include(PythonMITKModules.cmake)

add_subdirectory(SwigRuntime)

if(UNIX)
  set(MITK_CSWIG_LIBNAME_PREFIX "lib")
else(UNIX)
  set(MITK_CSWIG_LIBNAME_PREFIX "")
endif(UNIX)

set(MITK_CSWIG_PACKAGE_DIR_BUILD "${MITK_LIBRARY_PATH}")

#-----------------------------------------------------------------------------
# Configure python packages.
set(MITK_CSWIG_DATA_ROOT "'${MITK_DATA_ROOT}'")
set(MITK_CSWIG_TEST_ROOT "'${MITK_BINARY_DIR}/Testing'")
#if(MITK_CSWIG_CONFIGURATION_TYPES)
  #foreach(config ${MITK_CSWIG_CONFIGURATION_TYPES})
    #set(ITK_CSWIG_PACKAGE_DIR "'${MITK_CSWIG_PACKAGE_DIR_BUILD}/${config}'")
    #configure_file("${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/mitkbase.py.in"
                   #"${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${config}/mitkbase.py"
                   #@ONLY IMMEDIATE)
  #endforeach(config)
#else(MITK_CSWIG_CONFIGURATION_TYPES)
  #set(MITK_CSWIG_PACKAGE_DIR "'${MITK_CSWIG_PACKAGE_DIR_BUILD}'")
  #configure_file("${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/mitkbase.py.in"
                 #"${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitkbase.py"
                 #@ONLY IMMEDIATE)
#endif(MITK_CSWIG_CONFIGURATION_TYPES)

# Handle out-of-source builds correctly.
#
#  1. Create a list of Python files to be installed/copied.
#  2. Copy them to ITK_BINARY_DIR if it is different from ITK_SOURCE_DIR.
#  3. Use Python's compileall to compile the copied files.
#
# *** Step 1 has to be done carefully to avoid missing out files ***
if(PYTHON_EXECUTABLE AND MITK_USE_Python)

  #file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitk.py "")
  #configure_file(Python/mitk.py.in ${CMAKE_CURRENT_BINARY_DIR}/mitk.py)
  file(COPY Python/mitkCast.py DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)


  set(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/CMake)
  add_custom_target(mitkpython_pyc ALL echo "...")

  # Make the necessary directories.
  make_directory(${MITK_BINARY_DIR}/Wrapping/CSwig/Python)

  # Now create a list of Python files.
  set(MITK_PYTHON_FILES)

  # Wrapping/CSwig/Python/*.py
  set(MITK_PYTHON_FILES
    ${MITK_PYTHON_FILES}
    mitk
    )
  # Done listing files.

  # Now copy these files if necessary.
  set(MITK_PYTHON_SOURCE_FILES)
  set(MITK_PYTHON_OUTPUT_FILES)
  #if(MITK_CSWIG_CONFIGURATION_TYPES)
    #foreach(file ${MITK_PYTHON_FILES})
      #set(src "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${CMAKE_CFG_INTDIR}/${file}.py")
      #set(MITK_PYTHON_SOURCE_FILES ${MITK_PYTHON_SOURCE_FILES} ${src})
    #endforeach(file)
  #else(MITK_CSWIG_CONFIGURATION_TYPES)
    #foreach(file ${MITK_PYTHON_FILES})
      #set(src "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${file}.py")
      #set(ITK_PYTHON_SOURCE_FILES ${MITK_PYTHON_SOURCE_FILES} ${src})
    #endforeach(file)
  #endif(MITK_CSWIG_CONFIGURATION_TYPES)
  #if("${MITK_BINARY_DIR}" MATCHES "^${MITK_SOURCE_DIR}$")
    #message("In source build -- no need to copy Python files.")
  #else("${MITK_BINARY_DIR}" MATCHES "^${MITK_SOURCE_DIR}$")
    #if(MITK_CSWIG_CONFIGURATION_TYPES)
      #foreach(file ${MITK_PYTHON_FILES})
  #set(src "${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/${file}.py")
  #set(tgt "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file}.py")
  #add_custom_command(SOURCE ${src}
                           #COMMAND ${CMAKE_COMMAND}
                           #ARGS -E copy ${src} ${tgt}
                           #OUTPUTS ${tgt}
                           #TARGET copy
                           #COMMENT "source copy")
      #endforeach(file)
    #else(MITK_CSWIG_CONFIGURATION_TYPES)
      #foreach(file ${MITK_PYTHON_FILES})
        #set(src "${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/${file}.py")
        #set(tgt "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${file}.py")
        #add_custom_command(SOURCE ${src}
              #COMMAND ${CMAKE_COMMAND}
                          #ARGS -E copy ${src} ${tgt}
                          #OUTPUTS ${tgt}
        #TARGET mitkpython_pyc
                          #COMMENT "source copy")
      #endforeach(file)
    #endif(MITK_CSWIG_CONFIGURATION_TYPES)
  #endif("${MITK_BINARY_DIR}" MATCHES "^${MITK_SOURCE_DIR}$")

  # Byte compile the Python files.
  write_file(${MITK_BINARY_DIR}/Wrapping/CSwig/Python/compile_all_mitk
        "import compileall\n"
        "compileall.compile_dir('${MITK_BINARY_DIR}/Wrapping/CSwig/Python')\n"
  "file = open('${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitk_compile_complete', 'w')\n"
  "file.write('Done')\n")

  add_custom_command(
    SOURCE ${MITK_BINARY_DIR}/Wrapping/CSwig/Python/compile_all_mitk
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${MITK_BINARY_DIR}/Wrapping/CSwig/Python/compile_all_mitk
    DEPENDS ${MITK_PYTHON_SOURCE_FILES}
    OUTPUTS "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitk_compile_complete"
    TARGET mitkpython_pyc
    )

  add_custom_command(
    SOURCE mitkpython_pyc
    DEPENDS "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitk_compile_complete"
    TARGET mitkpython_pyc
    )

endif(PYTHON_EXECUTABLE AND MITK_USE_Python)

if(MITK_USE_Python)
  # Install the python files created at build time.
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4")
    # Old-style installation.
    configure_file(
      "${MITK_CSWIG_SOURCE_DIR}/pythonfiles_install.cmake.in"
      "${MITK_CSWIG_BINARY_DIR}/pythonfiles_install.cmake"
      @ONLY IMMEDIATE)
    add_custom_target(python_install)
    set_target_properties(python_install PROPERTIES
      POST_INSTALL_SCRIPT "${MITK_CSWIG_BINARY_DIR}/pythonfiles_install.cmake")
  else("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4")
    # Use CMake 2.6 FILES_MATCHING feature to install correctly.
    if(MITK_CSWIG_CONFIGURATION_TYPES)
      set(_FROMDIR ${EXECUTABLE_OUTPUT_PATH}/\${CMAKE_INSTALL_CONFIG_NAME})
    else(MITK_CSWIG_CONFIGURATION_TYPES)
      set(_FROMDIR ${EXECUTABLE_OUTPUT_PATH})
    endif(MITK_CSWIG_CONFIGURATION_TYPES)
  if(MITK_INSTALL_LIBRARY_DIR)
    install(
      DIRECTORY ${_FROMDIR}/
      DESTINATION ${MITK_INSTALL_LIBRARY_DIR}
      COMPONENT "RuntimeLibraries"
      FILES_MATCHING PATTERN "*.py"
      )
  endif(MITK_INSTALL_LIBRARY_DIR)
  endif("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4")

  # Install the package python files.
  #foreach(file ${MITK_PYTHON_FILES})
    #if(MITK_CSWIG_CONFIGURATION_TYPES)
      #install(FILES "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/\${CMAKE_INSTALL_CONFIG_NAME}/${file}.py"
        #DESTINATION ${MITK_INSTALL_LIBRARY_DIR}/python
        #COMPONENT RuntimeLibraries)
    #else(MITK_CSWIG_CONFIGURATION_TYPES)
      #install(FILES "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${file}.py"
        #DESTINATION ${MITK_INSTALL_LIBRARY_DIR}/python
        #COMPONENT RuntimeLibraries)
    #endif(MITK_CSWIG_CONFIGURATION_TYPES)
  #endforeach(file)

  set(MITK_CSWIG_PACKAGE_DIR "os.path.dirname(selfpath)")
  set(MITK_CSWIG_DATA_ROOT "os.path.join(os.path.dirname(selfpath),'Data')")
  set(MITK_CSWIG_TEST_ROOT "'<NO_DEFAULT>'")
  #configure_file("${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/mitkbase.py.in"
                 #"${MITK_BINARY_DIR}/Wrapping/CSwig/Python/InstallOnly/mitkbase.py"
                 #@ONLY IMMEDIATE)
  #install(FILES "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/InstallOnly/mitkbase.py"
    #DESTINATION ${MITK_INSTALL_LIBRARY_DIR}/python
    #COMPONENT RuntimeLibraries)
endif(MITK_USE_Python)


macro(MITK_WRAP_LIBRARY WRAP_SOURCES LIBRARY_NAME DIRECTORY DEPEND_LIBRARY EXTRA_SOURCES MITK_LINK_LIBRARIES)
  # loop over cable config files creating two lists:
  # WRAP_PYTHON_SOURCES: list of generated files
  foreach(Source ${WRAP_SOURCES})
    set(WRAP_PYTHON_SOURCES ${WRAP_PYTHON_SOURCES} ${Source}Python.cxx)
    set(ALL_IDX_FILES ${ALL_IDX_FILES} ${MITK_WRAP_BINARY_DIR}/${DIRECTORY}/${Source}.idx )
    set(INDEX_FILE_CONTENT "${INDEX_FILE_CONTENT}${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}/${Source}.idx\n")
  endforeach(Source)
  # add the package wrappers
  set(WRAP_PYTHON_SOURCES ${WRAP_PYTHON_SOURCES} wrap_${LIBRARY_NAME}PythonPython.cxx)
  if(MITK_EXTRA_PYTHON_WRAP)
    foreach( extraPython ${MITK_EXTRA_PYTHON_WRAP})
      set(WRAP_PYTHON_SOURCES ${WRAP_PYTHON_SOURCES} ${extraPython}Python.cxx)
    endforeach( extraPython )
  endif(MITK_EXTRA_PYTHON_WRAP)

  # set the generated sources as generated
  set_source_files_properties(
    ${WRAP_PYTHON_SOURCES}
    GENERATED )
  set(EXTRA_LIBS ${MITK_LINK_LIBRARIES})
  if("${MITK_LINK_LIBRARIES}" MATCHES "^$")
    #set(EXTRA_LIBS ${LIBRARY_NAME})
  endif("${MITK_LINK_LIBRARIES}" MATCHES "^$")

  file(APPEND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitk.py
        "from ${LIBRARY_NAME}Python import *\n")

  set(DEPENDENCIES mitkIpPic mbilog tinyxml)
  set(DEPENDS ${DEPENDENCIES})
  MITK_CHECK_MODULE(_MISSING_DEP ${DEPENDENCIES})
  MITK_USE_MODULE("Mitk")

  set(ALL_INCLUDE_DIRECTORIES ${ALL_INCLUDE_DIRECTORIES} ${MITK_INCLUDE_DIRS} ${MITK_WRAP_SOURCE_DIR} ${GDCM_INCLUDE_DIRS} )
  #  "D:/MITK/mitk/CoreUI/Bundles/org.mitk.gui.qt.common/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.ui.qt/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.ui/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.core.runtime/src" "D:/MITK/mitk/CoreUI/Bundles/org.mitk.gui.common/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.osgi/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.osgi/src/service" "D:/MITK/mitk/Utilities/Poco/Foundation/include" "D:/binPython/BlueBerry/Bundles/org.blueberry.osgi" "D:/MITK/mitk/Utilities/Poco/Util/include" "V:/windows/x32/QT-4.6.2_VC9.0_Bin/include/QtGui" "V:/windows/x32/QT-4.6.2_VC9.0_Bin/include"
  include_directories(${ALL_INCLUDE_DIRECTORIES})


  if(MITK_USE_Python)
    if(MITK_SWIG_FILE)
      set(SWIG_INC ${SWIG_INC} ${PYTHON_INCLUDE_PATH})
      set_source_files_properties(${MITK_SWIG_FILE_CXX}Python.cxx GENERATED)
      set(WRAP_FILE ${MITK_SWIG_FILE_CXX}Python.cxx )
    endif(MITK_SWIG_FILE)

    add_library(_${LIBRARY_NAME}Python SHARED
      ${WRAP_PYTHON_SOURCES}
      ${MITK_EXTRA_PYTHON_SOURCES}
      ${WRAP_FILE}
      ${EXTRA_SOURCES})

  #target_link_libraries(_${LIBRARY_NAME}Python ${DEPENDENCIES})

    if(MITK_WRAP_NEEDS_DEPEND)
      foreach(lib ${DEPEND_LIBRARY})
        add_dependencies(_${LIBRARY_NAME}Python _${lib}Python)
      endforeach(lib)
    endif(MITK_WRAP_NEEDS_DEPEND)
    if(MITK_LIBRARY_PROPERTIES)
      set_target_properties( _${LIBRARY_NAME}Python PROPERTIES PREFIX "" ${MITK_LIBRARY_PROPERTIES})
    else(MITK_LIBRARY_PROPERTIES)
  set_target_properties( _${LIBRARY_NAME}Python PROPERTIES PREFIX "")
    endif(MITK_LIBRARY_PROPERTIES)

    # Python extension modules on Windows must have the extension ".pyd"
    # instead of ".dll" as of Python 2.5.  Older python versions do support
    # this suffix.
    if(WIN32 AND NOT CYGWIN)
      set_target_properties(_${LIBRARY_NAME}Python PROPERTIES DEBUG_POSTFIX "_d" SUFFIX ".pyd")
    endif(WIN32 AND NOT CYGWIN)

    if(PYTHON_DEBUG_LIBRARY)
      target_link_libraries(_${LIBRARY_NAME}Python ${DEPENDENCIES} ${EXTRA_LIBS} SwigRuntimePython debug ${PYTHON_DEBUG_LIBRARY} optimized ${PYTHON_LIBRARY})
    else(PYTHON_DEBUG_LIBRARY)
    target_link_libraries(_${LIBRARY_NAME}Python ${DEPENDENCIES} ${EXTRA_LIBS} SwigRuntimePython ${PYTHON_LIBRARY})
    endif(PYTHON_DEBUG_LIBRARY)
  if(MITK_INSTALL_LIBRARY_DIR)
    install(TARGETS _${LIBRARY_NAME}Python
      RUNTIME DESTINATION ${MITK_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${MITK_INSTALL_LIBRARY_DIR} COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION ${MITK_INSTALL_LIBRARY_DIR} COMPONENT Development)
  endif(MITK_INSTALL_LIBRARY_DIR)
    if(MITK_SWIG_FILE)
      add_custom_command(
        COMMENT "run native swig on ${MITK_SWIG_FILE}"
        SOURCE ${MITK_SWIG_FILE}
        COMMAND ${CSWIG}
        ARGS -nocable -noruntime ${IGNORE_WARNINGS} -o ${WRAP_FILE}
         -outdir "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}"
         -python -c++ ${MITK_SWIG_FILE}
        TARGET _${LIBRARY_NAME}Python
        OUTPUTS ${WRAP_FILE}
        DEPENDS ${MITK_SWIG_FILE} ${CSWIG})
    endif(MITK_SWIG_FILE)
  endif(MITK_USE_Python)

  configure_file(
    ${MITK_CSWIG_SOURCE_DIR}/Master.mdx.in
    ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}/${LIBRARY_NAME}.mdx IMMEDIATE
    )

  set(SWIG_INC_FILE ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}/SwigInc.txt)
  set(SWIG_INC_CONTENTS)
  set(SWIG_INC ${ALL_INCLUDE_DIRECTORIES})
  foreach(dir ${SWIG_INC})
    set(SWIG_INC_CONTENTS "${SWIG_INC_CONTENTS}-I${dir}\n")
  endforeach(dir)
  configure_file(${MITK_CSWIG_SOURCE_DIR}/SwigInc.txt.in ${SWIG_INC_FILE}
    @ONLY IMMEDIATE)

  foreach(Source ${WRAP_SOURCES})

    if(MITK_USE_Python)
      # python
      WRAP_PYTHON_SOURCES(${MITK_CSWIG_SOURCE_DIR}/${DIRECTORY} ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}
        ${Source} _${LIBRARY_NAME}Python "${MASTER_INDEX_FILES}" "${ALL_IDX_FILES}")
    endif(MITK_USE_Python)

  endforeach(Source)


  # wrap the package files for python
  if(MITK_USE_Python)
    # python
    WRAP_PYTHON_SOURCES(${MITK_CSWIG_SOURCE_DIR}/${DIRECTORY} ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}
      wrap_${LIBRARY_NAME}Python _${LIBRARY_NAME}Python "${MASTER_INDEX_FILES}" "${ALL_IDX_FILES}")
    if(MITK_EXTRA_PYTHON_WRAP)
      foreach( extraPython ${MITK_EXTRA_PYTHON_WRAP})
        WRAP_PYTHON_SOURCES(${MITK_CSWIG_SOURCE_DIR}/${DIRECTORY} ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}
          ${extraPython} _${LIBRARY_NAME}Python "${MASTER_INDEX_FILES}" "${ALL_IDX_FILES}")
      endforeach( extraPython )
    endif(MITK_EXTRA_PYTHON_WRAP)

  endif(MITK_USE_Python)

endmacro(MITK_WRAP_LIBRARY)
