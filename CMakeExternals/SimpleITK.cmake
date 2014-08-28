#-----------------------------------------------------------------------------
# SimpleITK
#-----------------------------------------------------------------------------

if(MITK_USE_SimpleITK)

 # Sanity checks
 if(DEFINED SimpleITK_DIR AND NOT EXISTS ${SimpleITK_DIR})
   message(FATAL_ERROR "SimpleITK_DIR variable is defined but corresponds to non-existing directory")
 endif()

  set(proj SimpleITK)
  set(proj_DEPENDENCIES ITK GDCM Swig)

  if(MITK_USE_OpenCV)
    list(APPEND proj_DEPENDENCIES OpenCV)
  endif()

  set(SimpleITK_DEPENDS ${proj})

  if(NOT DEFINED SimpleITK_DIR)

    set(additional_cmake_args )

    list(APPEND additional_cmake_args
         -DWRAP_CSHARP:BOOL=OFF
         -DWRAP_TCL:BOOL=OFF
         -DWRAP_LUA:BOOL=OFF
         -DWRAP_PYTHON:BOOL=OFF
         -DWRAP_JAVA:BOOL=OFF
        )

    if(MITK_USE_Python)
      list(APPEND additional_cmake_args
           -DWRAP_PYTHON:BOOL=ON
           -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
           -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
           -DPYTHON_INCLUDE_DIR2:PATH=${PYTHON_INCLUDE_DIR2}
           -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
          )
      if(NOT MITK_USE_SYSTEM_PYTHON)
        list(APPEND proj_DEPENDENCIES Python)
      endif()
    else(MITK_USE_Python)
      list(APPEND additional_cmake_args
           -DWRAP_PYTHON:BOOL=OFF
          )
    endif()

    #TODO: Installer and testing works only with static libs on MAC
    set(_build_shared ON)
    if(APPLE)
      set(_build_shared OFF)
    endif()

    set(SimpleITK_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchSimpleITK.cmake)

    ExternalProject_Add(${proj}
       URL "https://dl.dropboxusercontent.com/u/8367205/ExternalProjects/SimpleITK.tar.gz"
       URL_MD5 "7cfa5d0ff79a540f4bcfaf992abe44d2"
       #GIT_TAG "493f15f5cfc620413d0aa7bb705ffe6d038a41b5"
       SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
       BINARY_DIR ${proj}-build
       PREFIX ${proj}-cmake
       INSTALL_DIR ${proj}-install
       PATCH_COMMAND ${SimpleITK_PATCH_COMMAND}
       CMAKE_ARGS
         ${ep_common_args}
         # -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
      CMAKE_CACHE_ARGS
         ${additional_cmake_args}
         -DBUILD_SHARED_LIBS:BOOL=${_build_shared}
         -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
         -DCMAKE_INSTALL_NAME_DIR:STRING=<INSTALL_DIR>/lib
         -DSimpleITK_BUILD_DISTRIBUTE:BOOL=ON
         -DSimpleITK_PYTHON_THREADS:BOOL=ON
         -DUSE_SYSTEM_ITK:BOOL=ON
         -DBUILD_TESTING:BOOL=OFF
         -DBUILD_EXAMPLES:BOOL=OFF
         -DGDCM_DIR:PATH=${GDCM_DIR}
         -DITK_DIR:PATH=${ITK_DIR}
         -DSWIG_DIR:PATH=${SWIG_DIR}
         -DSWIG_EXECUTABLE:FILEPATH=${SWIG_EXECUTABLE}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(SimpleITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

    if( MITK_USE_Python )
      # Build python distribution with easy install. If a own runtime is used
      # embedd the egg into the site-package folder of the runtime
      if(NOT MITK_USE_SYSTEM_PYTHON)
        ExternalProject_Add_Step(${proj} sitk_python_install_step
          COMMAND ${PYTHON_EXECUTABLE} setup.py install --prefix=${Python_DIR}
          DEPENDEES build
          WORKING_DIRECTORY ${SimpleITK_DIR}/Wrapping/PythonPackage
        )
      # Build egg into custom user base folder and deploy it later into installer
      # https://pythonhosted.org/setuptools/easy_install.html#use-the-user-option-and-customize-pythonuserbase
      else()
        set(_userbase_install ${SimpleITK_DIR}/Wrapping/PythonPackage/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
        if(WIN32)
          set(_userbase_install ${SimpleITK_DIR}/Wrapping/PythonPackage/Lib/site-packages)
        endif()
        ExternalProject_Add_Step(${proj} sitk_create_userbase_step
          COMMAND ${CMAKE_COMMAND} -E make_directory ${_userbase_install}
          DEPENDEES build
          WORKING_DIRECTORY ${SimpleITK_DIR}/Wrapping/PythonPackage
        )
        ExternalProject_Add_Step(${proj} sitk_python_install_step
          COMMAND PYTHONUSERBASE=${SimpleITK_DIR}/Wrapping/PythonPackage ${PYTHON_EXECUTABLE} setup.py install --user
          DEPENDEES sitk_create_userbase_step
          WORKING_DIRECTORY ${SimpleITK_DIR}/Wrapping/PythonPackage
        )
      endif()
    endif()

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()

