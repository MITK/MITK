IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

IF(MITK_USE_Python)
  # Python include directory.
  SET(MITK_INCLUDE_DIRS ${MITK_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_PATH})
ENDIF(MITK_USE_Python)

# We have found CableSwig.  Use the settings.
SET(CABLE_INDEX ${CableSwig_cableidx_EXE})
SET(CSWIG ${CableSwig_cswig_EXE})
SET(GCCXML ${CableSwig_gccxml_EXE})
SET(MITK_WRAPPING_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(MITK_WRAPPING_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

#SET(MITK_WRAP_NEEDS_DEPEND 1)
#IF(${CMAKE_MAKE_PROGRAM} MATCHES make)
  #SET(MITK_WRAP_NEEDS_DEPEND 0)
#ENDIF(${CMAKE_MAKE_PROGRAM} MATCHES make)
SET(MITK_TOP ${MITK_SOURCE_DIR})
SET(MITK_SWIG_DEFAULT_LIB ${CableSwig_DIR}/Swig/Lib )
SET(MITK_WRAP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(MITK_WRAP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

SET(CSWIG_EXTRA_LINKFLAGS )
IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  SET(CSWIG_EXTRA_LINKFLAGS "/IGNORE:4049")
ENDIF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")

IF(CMAKE_SYSTEM MATCHES "IRIX.*")
  IF(CMAKE_CXX_COMPILER MATCHES "CC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -woff 1552")
  ENDIF(CMAKE_CXX_COMPILER MATCHES "CC")
ENDIF(CMAKE_SYSTEM MATCHES "IRIX.*")

IF(CMAKE_COMPILER_IS_GNUCXX)
  STRING(REGEX REPLACE "-Wcast-qual" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

SET(IGNORE_WARNINGS -w362 -w389 -w503 -w508 -w509 -w516)
# define macros for wrapping commands
MACRO(GCCXML_CREATE_XML_FILE Source Bin Input Output Library)
# if the make program is not an IDE then include
# the depend file in a way that will make cmake
# re-run if it changes
   SET(CABLE_SWIG_DEPEND)
   SET(CABLE_SWIG_DEPEND_REGENERATE)
   IF(${CMAKE_MAKE_PROGRAM} MATCHES "make")
     IF(EXISTS ${Bin}/${Output}.depend)
     ELSE(EXISTS ${Bin}/${Output}.depend)
       CONFIGURE_FILE(
         ${MITK_SOURCE_DIR}/Wrapping/CSwig/empty.depend.in
         ${Bin}/${Output}.depend @ONLY IMMEDIATE)
     ENDIF(EXISTS ${Bin}/${Output}.depend)
     INCLUDE(${Bin}/${Output}.depend)
   ELSE(${CMAKE_MAKE_PROGRAM} MATCHES "make")
# for IDE generators like MS dev only include the depend files
# if they exist.   This is to prevent ecessive reloading of
# workspaces after each build.   This also means
# that the depends will not be correct until cmake
# is run once after the build has completed once.
# the depend files are created in the wrap tcl/python sections
# when the .xml file is parsed.
     INCLUDE(${Bin}/${Output}.depend OPTIONAL)
   ENDIF(${CMAKE_MAKE_PROGRAM} MATCHES "make")

   IF(CABLE_SWIG_DEPEND)
     # There are dependencies.  Make sure all the files are present.
     # If not, force the rule to re-run to update the dependencies.
     FOREACH(f ${CABLE_SWIG_DEPEND})
       IF(EXISTS ${f})
       ELSE(EXISTS ${f})
         SET(CABLE_SWIG_DEPEND_REGENERATE 1)
       ENDIF(EXISTS ${f})
     ENDFOREACH(f)
   ELSE(CABLE_SWIG_DEPEND)
     # No dependencies, make the output depend on the dependency file
     # itself, which should cause the rule to re-run.
     SET(CABLE_SWIG_DEPEND_REGENERATE 1)
   ENDIF(CABLE_SWIG_DEPEND)
   IF(CABLE_SWIG_DEPEND_REGENERATE)
     SET(CABLE_SWIG_DEPEND ${Bin}/${Output}.depend)
     CONFIGURE_FILE(
       ${MITK_SOURCE_DIR}/Wrapping/CSwig/empty.depend.in
       ${Bin}/${Output}.depend @ONLY IMMEDIATE)
   ENDIF(CABLE_SWIG_DEPEND_REGENERATE)
  SET(OutDir "Debug")

   IF(WIN32)
   SET(OS -DWIN32)
   ENDIF(WIN32)

   ADD_CUSTOM_COMMAND(
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

ENDMACRO(GCCXML_CREATE_XML_FILE)


MACRO(CINDEX_CREATE_IDX_FILE Bin Input Output Library)
   ADD_CUSTOM_COMMAND(
   COMMENT "${Output} from "
   SOURCE ${Bin}/${Input}
   COMMAND ${CABLE_INDEX}
   ARGS ${Bin}/${Input} ${Bin}/${Output}
   TARGET ${Library}
   OUTPUTS ${Bin}/${Output}
   DEPENDS ${Bin}/${Input} ${CABLE_INDEX}
)
ENDMACRO(CINDEX_CREATE_IDX_FILE)

SET(MITK_USE_Python_NO_EXCEPTION_REGEX "(ContinuousIndex|Python)\\.xml$")

MACRO(CSWIG_CREATE_PYTHON_CXX_FILE Bin MasterIdx InputIdx InputXml OutputPythonCxx Library LibraryIndexFiles)
   SET(CINDEX)
   FOREACH(MIDX ${MasterIdx})
     SET(CINDEX ${CINDEX} -Cindex "${MIDX}")
   ENDFOREACH(MIDX)
   IF("${InputXml}" MATCHES "${MITK_USE_Python_NO_EXCEPTION_REGEX}")
     SET(MITK_SWG_FILE "")
   ELSE("${InputXml}" MATCHES "${MITK_USE_Python_NO_EXCEPTION_REGEX}")
     SET(MITK_SWG_FILE "-l${MITK_CSWIG_SOURCE_DIR}/mitk.swg" "-l${MITK_CSWIG_SOURCE_DIR}/mitkvtk.swg")
   ENDIF("${InputXml}" MATCHES "${MITK_USE_Python_NO_EXCEPTION_REGEX}")
   STRING(REPLACE "_" "" LIB_PYTHON_FILES ${Library})

   #MESSAGE(STATUS "CMAKE_CFG_INTDIR: ${CMAKE_CFG_INTDIR}")
   #MESSAGE(STATUS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${LIB_PYTHON_FILES}")
   #MAKE_DIRECTORY(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${LIB_PYTHON_FILES})
   MAKE_DIRECTORY(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIB_PYTHON_FILES})
   FILE(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${LIB_PYTHON_FILES}/__init__.py "from mitkCorePython import *")

   ADD_CUSTOM_COMMAND(
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
ENDMACRO(CSWIG_CREATE_PYTHON_CXX_FILE)

# macro to create .xml, .idx and Python.cxx files
MACRO(WRAP_PYTHON_SOURCES Source Bin BaseName LibraryName MasterIdx LibraryIndexFiles)
   GCCXML_CREATE_XML_FILE(${Source} ${Bin} ${BaseName}.cxx ${BaseName}.xml ${LibraryName})
   CINDEX_CREATE_IDX_FILE(${Bin} ${BaseName}.xml  ${BaseName}.idx ${LibraryName})
   CSWIG_CREATE_PYTHON_CXX_FILE(${Bin} "${MasterIdx}" ${BaseName}.idx ${BaseName}.xml
                             ${BaseName}Python.cxx ${LibraryName} "${LibraryIndexFiles}")
ENDMACRO(WRAP_PYTHON_SOURCES)

# make sure required stuff is set
IF(NOT EXECUTABLE_OUTPUT_PATH)
  SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)

SET(MITK_CSWIG_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
SET(MITK_CSWIG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

IF(MITK_LIBRARY_DIRS)
  STRING(REGEX REPLACE "^/" "" MITK_INSTALL_LIBRARY_DIR "${MITK_LIBRARY_DIRS}")
ENDIF(MITK_LIBRARY_DIRS)
IF(MITK_BINARY_DIR)
  STRING(REGEX REPLACE "^/" "" MITK_INSTALL_BIN_DIR "${MITK_BINARY_DIR}")
ENDIF(MITK_BINARY_DIR)
SET(CSWIG_MISSING_VALUES)
IF(NOT CSWIG)
   SET(CSWIG_MISSING_VALUES "${CSWIG_MISSING_VALUES} CSWIG ")
ENDIF(NOT CSWIG)
IF(NOT CABLE_INDEX)
   SET(CSWIG_MISSING_VALUES "${CSWIG_MISSING_VALUES} CABLE_INDEX ")
ENDIF(NOT CABLE_INDEX)
IF(NOT GCCXML)
   SET(CSWIG_MISSING_VALUES "${CSWIG_MISSING_VALUES} GCCXML ")
ENDIF(NOT GCCXML)
IF(CSWIG_MISSING_VALUES)
  MESSAGE(SEND_ERROR "To use cswig wrapping, CSWIG, CABLE_INDEX, and GCCXML executables must be specified.  If they are all in the same directory, only specifiy one of them, and then run cmake configure again and the others should be found.\nCurrently, you are missing the following:\n ${CSWIG_MISSING_VALUES}")
ENDIF(CSWIG_MISSING_VALUES)


IF(MITK_USE_Python)
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH} )
ENDIF(MITK_USE_Python)

SET(SWIG_INC
  ${MITK_INCLUDE_DIRS_BUILD_TREE}
  ${MITK_INCLUDE_DIRS_BUILD_TREE_CXX}
  ${MITK_INCLUDE_DIRS}
  ${MITK_TOP}/Wrapping/CSwig
  ${MITK_TOP}/Wrapping/CSwig/Core
  )
ADD_DEFINITIONS(-DSWIG_GLOBAL)

INCLUDE(PythonMITKModules.cmake)

ADD_SUBDIRECTORY(SwigRuntime)

IF(UNIX)
  SET(MITK_CSWIG_LIBNAME_PREFIX "lib")
ELSE(UNIX)
  SET(MITK_CSWIG_LIBNAME_PREFIX "")
ENDIF(UNIX)

SET(MITK_CSWIG_PACKAGE_DIR_BUILD "${MITK_LIBRARY_PATH}")

#-----------------------------------------------------------------------------
# Configure python packages.
SET(MITK_CSWIG_DATA_ROOT "'${MITK_DATA_ROOT}'")
SET(MITK_CSWIG_TEST_ROOT "'${MITK_BINARY_DIR}/Testing'")
#IF(MITK_CSWIG_CONFIGURATION_TYPES)
  #FOREACH(config ${MITK_CSWIG_CONFIGURATION_TYPES})
    #SET(ITK_CSWIG_PACKAGE_DIR "'${MITK_CSWIG_PACKAGE_DIR_BUILD}/${config}'")
    #CONFIGURE_FILE("${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/mitkbase.py.in"
                   #"${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${config}/mitkbase.py"
                   #@ONLY IMMEDIATE)
  #ENDFOREACH(config)
#ELSE(MITK_CSWIG_CONFIGURATION_TYPES)
  #SET(MITK_CSWIG_PACKAGE_DIR "'${MITK_CSWIG_PACKAGE_DIR_BUILD}'")
  #CONFIGURE_FILE("${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/mitkbase.py.in"
                 #"${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitkbase.py"
                 #@ONLY IMMEDIATE)
#ENDIF(MITK_CSWIG_CONFIGURATION_TYPES)

# Handle out-of-source builds correctly.
#
#  1. Create a list of Python files to be installed/copied.
#  2. Copy them to ITK_BINARY_DIR if it is different from ITK_SOURCE_DIR.
#  3. Use Python's compileall to compile the copied files.
#
# *** Step 1 has to be done carefully to avoid missing out files ***
IF(PYTHON_EXECUTABLE AND MITK_USE_Python)

  #FILE(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitk.py "")
  #CONFIGURE_FILE(Python/mitk.py.in ${CMAKE_CURRENT_BINARY_DIR}/mitk.py)
  FILE(COPY Python/mitkCast.py DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)


  SET(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/CMake)
  ADD_CUSTOM_TARGET(mitkpython_pyc ALL echo "...")

  # Make the necessary directories.
  MAKE_DIRECTORY(${MITK_BINARY_DIR}/Wrapping/CSwig/Python)

  # Now create a list of Python files.
  SET(MITK_PYTHON_FILES)

  # Wrapping/CSwig/Python/*.py
  SET(MITK_PYTHON_FILES
    ${MITK_PYTHON_FILES}
    mitk
    )
  # Done listing files.

  # Now copy these files if necessary.
  SET(MITK_PYTHON_SOURCE_FILES)
  SET(MITK_PYTHON_OUTPUT_FILES)
  #IF(MITK_CSWIG_CONFIGURATION_TYPES)
    #FOREACH(file ${MITK_PYTHON_FILES})
      #SET(src "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${CMAKE_CFG_INTDIR}/${file}.py")
      #SET(MITK_PYTHON_SOURCE_FILES ${MITK_PYTHON_SOURCE_FILES} ${src})
    #ENDFOREACH(file)
  #ELSE(MITK_CSWIG_CONFIGURATION_TYPES)
    #FOREACH(file ${MITK_PYTHON_FILES})
      #SET(src "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${file}.py")
      #SET(ITK_PYTHON_SOURCE_FILES ${MITK_PYTHON_SOURCE_FILES} ${src})
    #ENDFOREACH(file)
  #ENDIF(MITK_CSWIG_CONFIGURATION_TYPES)
  #IF ("${MITK_BINARY_DIR}" MATCHES "^${MITK_SOURCE_DIR}$")
    #MESSAGE("In source build -- no need to copy Python files.")
  #ELSE ("${MITK_BINARY_DIR}" MATCHES "^${MITK_SOURCE_DIR}$")
    #IF(MITK_CSWIG_CONFIGURATION_TYPES)
      #FOREACH(file ${MITK_PYTHON_FILES})
  #SET(src "${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/${file}.py")
  #SET(tgt "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${file}.py")
  #ADD_CUSTOM_COMMAND(SOURCE ${src}
                           #COMMAND ${CMAKE_COMMAND}
                           #ARGS -E copy ${src} ${tgt}
                           #OUTPUTS ${tgt}
                           #TARGET copy
                           #COMMENT "source copy")
      #ENDFOREACH(file)
    #ELSE(MITK_CSWIG_CONFIGURATION_TYPES)
      #FOREACH(file ${MITK_PYTHON_FILES})
        #SET(src "${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/${file}.py")
        #SET(tgt "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${file}.py")
        #ADD_CUSTOM_COMMAND(SOURCE ${src}
              #COMMAND ${CMAKE_COMMAND}
                          #ARGS -E copy ${src} ${tgt}
                          #OUTPUTS ${tgt}
        #TARGET mitkpython_pyc
                          #COMMENT "source copy")
      #ENDFOREACH(file)
    #ENDIF(MITK_CSWIG_CONFIGURATION_TYPES)
  #ENDIF ("${MITK_BINARY_DIR}" MATCHES "^${MITK_SOURCE_DIR}$")

  # Byte compile the Python files.
  WRITE_FILE(${MITK_BINARY_DIR}/Wrapping/CSwig/Python/compile_all_mitk
        "import compileall\n"
        "compileall.compile_dir('${MITK_BINARY_DIR}/Wrapping/CSwig/Python')\n"
  "file = open('${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitk_compile_complete', 'w')\n"
  "file.write('Done')\n")

  ADD_CUSTOM_COMMAND(
    SOURCE ${MITK_BINARY_DIR}/Wrapping/CSwig/Python/compile_all_mitk
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${MITK_BINARY_DIR}/Wrapping/CSwig/Python/compile_all_mitk
    DEPENDS ${MITK_PYTHON_SOURCE_FILES}
    OUTPUTS "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitk_compile_complete"
    TARGET mitkpython_pyc
    )

  ADD_CUSTOM_COMMAND(
    SOURCE mitkpython_pyc
    DEPENDS "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/mitk_compile_complete"
    TARGET mitkpython_pyc
    )

ENDIF(PYTHON_EXECUTABLE AND MITK_USE_Python)

IF(MITK_USE_Python)
  # Install the python files created at build time.
  IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4")
    # Old-style installation.
    CONFIGURE_FILE(
      "${MITK_CSWIG_SOURCE_DIR}/pythonfiles_install.cmake.in"
      "${MITK_CSWIG_BINARY_DIR}/pythonfiles_install.cmake"
      @ONLY IMMEDIATE)
    ADD_CUSTOM_TARGET(python_install)
    SET_TARGET_PROPERTIES(python_install PROPERTIES
      POST_INSTALL_SCRIPT "${MITK_CSWIG_BINARY_DIR}/pythonfiles_install.cmake")
  ELSE("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4")
    # Use CMake 2.6 FILES_MATCHING feature to install correctly.
    IF(MITK_CSWIG_CONFIGURATION_TYPES)
      SET(_FROMDIR ${EXECUTABLE_OUTPUT_PATH}/\${CMAKE_INSTALL_CONFIG_NAME})
    ELSE(MITK_CSWIG_CONFIGURATION_TYPES)
      SET(_FROMDIR ${EXECUTABLE_OUTPUT_PATH})
    ENDIF(MITK_CSWIG_CONFIGURATION_TYPES)
  IF(MITK_INSTALL_LIBRARY_DIR)
    INSTALL(
      DIRECTORY ${_FROMDIR}/
      DESTINATION ${MITK_INSTALL_LIBRARY_DIR}
      COMPONENT "RuntimeLibraries"
      FILES_MATCHING PATTERN "*.py"
      )
  ENDIF(MITK_INSTALL_LIBRARY_DIR)
  ENDIF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" STREQUAL "2.4")

  # Install the package python files.
  #FOREACH(file ${MITK_PYTHON_FILES})
    #IF(MITK_CSWIG_CONFIGURATION_TYPES)
      #INSTALL(FILES "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/\${CMAKE_INSTALL_CONFIG_NAME}/${file}.py"
        #DESTINATION ${MITK_INSTALL_LIBRARY_DIR}/python
        #COMPONENT RuntimeLibraries)
    #ELSE(MITK_CSWIG_CONFIGURATION_TYPES)
      #INSTALL(FILES "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/${file}.py"
        #DESTINATION ${MITK_INSTALL_LIBRARY_DIR}/python
        #COMPONENT RuntimeLibraries)
    #ENDIF(MITK_CSWIG_CONFIGURATION_TYPES)
  #ENDFOREACH(file)

  SET(MITK_CSWIG_PACKAGE_DIR "os.path.dirname(selfpath)")
  SET(MITK_CSWIG_DATA_ROOT "os.path.join(os.path.dirname(selfpath),'Data')")
  SET(MITK_CSWIG_TEST_ROOT "'<NO_DEFAULT>'")
  #CONFIGURE_FILE("${MITK_SOURCE_DIR}/Wrapping/CSwig/Python/mitkbase.py.in"
                 #"${MITK_BINARY_DIR}/Wrapping/CSwig/Python/InstallOnly/mitkbase.py"
                 #@ONLY IMMEDIATE)
  #INSTALL(FILES "${MITK_BINARY_DIR}/Wrapping/CSwig/Python/InstallOnly/mitkbase.py"
    #DESTINATION ${MITK_INSTALL_LIBRARY_DIR}/python
    #COMPONENT RuntimeLibraries)
ENDIF(MITK_USE_Python)


MACRO(MITK_WRAP_LIBRARY WRAP_SOURCES LIBRARY_NAME DIRECTORY DEPEND_LIBRARY EXTRA_SOURCES MITK_LINK_LIBRARIES)
  # loop over cable config files creating two lists:
  # WRAP_PYTHON_SOURCES: list of generated files
  FOREACH(Source ${WRAP_SOURCES})
    SET(WRAP_PYTHON_SOURCES ${WRAP_PYTHON_SOURCES} ${Source}Python.cxx)
    SET(ALL_IDX_FILES ${ALL_IDX_FILES} ${MITK_WRAP_BINARY_DIR}/${DIRECTORY}/${Source}.idx )
    SET(INDEX_FILE_CONTENT "${INDEX_FILE_CONTENT}${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}/${Source}.idx\n")
  ENDFOREACH(Source)
  # add the package wrappers
  SET(WRAP_PYTHON_SOURCES ${WRAP_PYTHON_SOURCES} wrap_${LIBRARY_NAME}PythonPython.cxx)
  IF(MITK_EXTRA_PYTHON_WRAP)
    FOREACH( extraPython ${MITK_EXTRA_PYTHON_WRAP})
      SET(WRAP_PYTHON_SOURCES ${WRAP_PYTHON_SOURCES} ${extraPython}Python.cxx)
    ENDFOREACH( extraPython )
  ENDIF(MITK_EXTRA_PYTHON_WRAP)

  # set the generated sources as generated
  SET_SOURCE_FILES_PROPERTIES(
    ${WRAP_PYTHON_SOURCES}
    GENERATED )
  SET(EXTRA_LIBS ${MITK_LINK_LIBRARIES})
  IF("${MITK_LINK_LIBRARIES}" MATCHES "^$")
    #SET(EXTRA_LIBS ${LIBRARY_NAME})
  ENDIF("${MITK_LINK_LIBRARIES}" MATCHES "^$")

  FILE(APPEND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitk.py
        "from ${LIBRARY_NAME}Python import *\n")

  SET(DEPENDENCIES mitkIpPic mbilog tinyxml)
  SET(DEPENDS ${DEPENDENCIES})
  MITK_CHECK_MODULE(_MISSING_DEP ${DEPENDENCIES})
  MITK_USE_MODULE("Mitk")

  SET(ALL_INCLUDE_DIRECTORIES ${ALL_INCLUDE_DIRECTORIES} ${MITK_INCLUDE_DIRS} ${MITK_WRAP_SOURCE_DIR} ${GDCM_INCLUDE_DIRS} )
  #  "D:/MITK/mitk/CoreUI/Bundles/org.mitk.gui.qt.common/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.ui.qt/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.ui/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.core.runtime/src" "D:/MITK/mitk/CoreUI/Bundles/org.mitk.gui.common/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.osgi/src" "D:/MITK/mitk/BlueBerry/Bundles/org.blueberry.osgi/src/service" "D:/MITK/mitk/Utilities/Poco/Foundation/include" "D:/binPython/BlueBerry/Bundles/org.blueberry.osgi" "D:/MITK/mitk/Utilities/Poco/Util/include" "V:/windows/x32/QT-4.6.2_VC9.0_Bin/include/QtGui" "V:/windows/x32/QT-4.6.2_VC9.0_Bin/include"
  INCLUDE_DIRECTORIES(${ALL_INCLUDE_DIRECTORIES})


  IF(MITK_USE_Python)
    IF(MITK_SWIG_FILE)
      SET(SWIG_INC ${SWIG_INC} ${PYTHON_INCLUDE_PATH})
      SET_SOURCE_FILES_PROPERTIES(${MITK_SWIG_FILE_CXX}Python.cxx GENERATED)
      SET(WRAP_FILE ${MITK_SWIG_FILE_CXX}Python.cxx )
    ENDIF(MITK_SWIG_FILE)

    ADD_LIBRARY(_${LIBRARY_NAME}Python SHARED
      ${WRAP_PYTHON_SOURCES}
      ${MITK_EXTRA_PYTHON_SOURCES}
      ${WRAP_FILE}
      ${EXTRA_SOURCES})

  #TARGET_LINK_LIBRARIES(_${LIBRARY_NAME}Python ${DEPENDENCIES})

    IF(MITK_WRAP_NEEDS_DEPEND)
      FOREACH(lib ${DEPEND_LIBRARY})
        ADD_DEPENDENCIES(_${LIBRARY_NAME}Python _${lib}Python)
      ENDFOREACH(lib)
    ENDIF(MITK_WRAP_NEEDS_DEPEND)
    IF(MITK_LIBRARY_PROPERTIES)
      SET_TARGET_PROPERTIES( _${LIBRARY_NAME}Python PROPERTIES PREFIX "" ${MITK_LIBRARY_PROPERTIES})
    ELSE(MITK_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES( _${LIBRARY_NAME}Python PROPERTIES PREFIX "")
    ENDIF(MITK_LIBRARY_PROPERTIES)

    # Python extension modules on Windows must have the extension ".pyd"
    # instead of ".dll" as of Python 2.5.  Older python versions do support
    # this suffix.
    IF(WIN32 AND NOT CYGWIN)
      SET_TARGET_PROPERTIES(_${LIBRARY_NAME}Python PROPERTIES DEBUG_POSTFIX "_d" SUFFIX ".pyd")
    ENDIF(WIN32 AND NOT CYGWIN)

    IF(PYTHON_DEBUG_LIBRARY)
      TARGET_LINK_LIBRARIES(_${LIBRARY_NAME}Python ${DEPENDENCIES} ${EXTRA_LIBS} SwigRuntimePython debug ${PYTHON_DEBUG_LIBRARY} optimized ${PYTHON_LIBRARY})
    ELSE(PYTHON_DEBUG_LIBRARY)
    TARGET_LINK_LIBRARIES(_${LIBRARY_NAME}Python ${DEPENDENCIES} ${EXTRA_LIBS} SwigRuntimePython ${PYTHON_LIBRARY})
    ENDIF(PYTHON_DEBUG_LIBRARY)
  IF(MITK_INSTALL_LIBRARY_DIR)
    INSTALL(TARGETS _${LIBRARY_NAME}Python
      RUNTIME DESTINATION ${MITK_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${MITK_INSTALL_LIBRARY_DIR} COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION ${MITK_INSTALL_LIBRARY_DIR} COMPONENT Development)
  ENDIF(MITK_INSTALL_LIBRARY_DIR)
    IF(MITK_SWIG_FILE)
      ADD_CUSTOM_COMMAND(
        COMMENT "run native swig on ${MITK_SWIG_FILE}"
        SOURCE ${MITK_SWIG_FILE}
        COMMAND ${CSWIG}
        ARGS -nocable -noruntime ${IGNORE_WARNINGS} -o ${WRAP_FILE}
         -outdir "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}"
         -python -c++ ${MITK_SWIG_FILE}
        TARGET _${LIBRARY_NAME}Python
        OUTPUTS ${WRAP_FILE}
        DEPENDS ${MITK_SWIG_FILE} ${CSWIG})
    ENDIF(MITK_SWIG_FILE)
  ENDIF(MITK_USE_Python)

  CONFIGURE_FILE(
    ${MITK_CSWIG_SOURCE_DIR}/Master.mdx.in
    ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}/${LIBRARY_NAME}.mdx IMMEDIATE
    )

  SET(SWIG_INC_FILE ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}/SwigInc.txt)
  SET(SWIG_INC_CONTENTS)
  SET(SWIG_INC ${ALL_INCLUDE_DIRECTORIES})
  FOREACH(dir ${SWIG_INC})
    SET(SWIG_INC_CONTENTS "${SWIG_INC_CONTENTS}-I${dir}\n")
  ENDFOREACH(dir)
  CONFIGURE_FILE(${MITK_CSWIG_SOURCE_DIR}/SwigInc.txt.in ${SWIG_INC_FILE}
    @ONLY IMMEDIATE)

  FOREACH(Source ${WRAP_SOURCES})

    IF(MITK_USE_Python)
      # python
      WRAP_PYTHON_SOURCES(${MITK_CSWIG_SOURCE_DIR}/${DIRECTORY} ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}
        ${Source} _${LIBRARY_NAME}Python "${MASTER_INDEX_FILES}" "${ALL_IDX_FILES}")
    ENDIF(MITK_USE_Python)

  ENDFOREACH(Source)


  # wrap the package files for python
  IF(MITK_USE_Python)
    # python
    WRAP_PYTHON_SOURCES(${MITK_CSWIG_SOURCE_DIR}/${DIRECTORY} ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}
      wrap_${LIBRARY_NAME}Python _${LIBRARY_NAME}Python "${MASTER_INDEX_FILES}" "${ALL_IDX_FILES}")
    IF(MITK_EXTRA_PYTHON_WRAP)
      FOREACH( extraPython ${MITK_EXTRA_PYTHON_WRAP})
        WRAP_PYTHON_SOURCES(${MITK_CSWIG_SOURCE_DIR}/${DIRECTORY} ${MITK_CSWIG_BINARY_DIR}/${DIRECTORY}
          ${extraPython} _${LIBRARY_NAME}Python "${MASTER_INDEX_FILES}" "${ALL_IDX_FILES}")
      ENDFOREACH( extraPython )
    ENDIF(MITK_EXTRA_PYTHON_WRAP)

  ENDIF(MITK_USE_Python)

ENDMACRO(MITK_WRAP_LIBRARY)
