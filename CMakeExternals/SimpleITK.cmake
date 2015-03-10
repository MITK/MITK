#-----------------------------------------------------------------------------
# SimpleITK
#-----------------------------------------------------------------------------

if(MITK_USE_SimpleITK)

 # Sanity checks
 if(DEFINED SimpleITK_DIR AND NOT EXISTS ${SimpleITK_DIR})
   message(FATAL_ERROR "SimpleITK_DIR variable is defined but corresponds to non-existing directory")
 endif()

  set(proj SimpleITK)
  set(proj_DEPENDENCIES ITK GDCM SWIG)

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
         -DWRAP_RUBY:BOOL=OFF
         -DWRAP_R:BOOL=OFF
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
    endif()

    #TODO: Installer and testing works only with static libs on MAC
    set(_build_shared ON)
    if(APPLE)
      set(_build_shared OFF)
    endif()

    ExternalProject_Add(${proj}
       LIST_SEPARATOR ${sep}
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/SimpleITK-0.8.1.tar.gz
       URL_MD5 9126ab2eda9e88f598a962c02a705c43
       PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/SimpleITK-0.8.1.patch
       CMAKE_ARGS
         ${ep_common_args}
         # -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
      CMAKE_CACHE_ARGS
         ${additional_cmake_args}
         -DBUILD_SHARED_LIBS:BOOL=${_build_shared}
         -DSimpleITK_BUILD_DISTRIBUTE:BOOL=ON
         -DSimpleITK_PYTHON_THREADS:BOOL=ON
         -DUSE_SYSTEM_ITK:BOOL=ON
         -DBUILD_EXAMPLES:BOOL=OFF
         -DGDCM_DIR:PATH=${GDCM_DIR}
         -DITK_DIR:PATH=${ITK_DIR}
         -DSWIG_DIR:PATH=${SWIG_DIR}
         -DSWIG_EXECUTABLE:FILEPATH=${SWIG_EXECUTABLE}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(SimpleITK_DIR ${ep_prefix})

    if( MITK_USE_Python AND NOT MITK_USE_SYSTEM_PYTHON )
      # PythonDir needs to be fixed for the python interpreter by
      # changing dir delimiter for Windows
      set(_install_dir ${Python_DIR})
      if(WIN32)
        STRING(REPLACE "/" "\\\\" _install_dir ${Python_DIR})
      else()
        # escape spaces in the install path for linux
        STRING(REPLACE " " "\ " _install_dir ${Python_DIR})
      endif()
      # Build python distribution with easy install. If a own runtime is used
      # embedd the egg into the site-package folder of the runtime
      if(NOT MITK_USE_SYSTEM_PYTHON)
        ExternalProject_Add_Step(${proj} sitk_python_install_step
          COMMAND ${PYTHON_EXECUTABLE} setup.py install --prefix=${_install_dir}
          DEPENDEES build
          WORKING_DIRECTORY <BINARY_DIR>/Wrapping/PythonPackage
        )
      # Build egg into custom user base folder and deploy it later into installer
      # https://pythonhosted.org/setuptools/easy_install.html#use-the-user-option-and-customize-pythonuserbase
      else()
        set(_userbase_install <BINARY_DIR>/Wrapping/PythonPackage/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
        if(WIN32)
          set(_userbase_install <BINARY_DIR>/Wrapping/PythonPackage/Lib/site-packages)
        endif()
        ExternalProject_Add_Step(${proj} sitk_create_userbase_step
          COMMAND ${CMAKE_COMMAND} -E make_directory ${_userbase_install}
          DEPENDEES build
          WORKING_DIRECTORY ${SimpleITK_DIR}/Wrapping/PythonPackage
        )
        ExternalProject_Add_Step(${proj} sitk_python_install_step
          COMMAND PYTHONUSERBASE=<BINARY_DIR>/Wrapping/PythonPackage ${PYTHON_EXECUTABLE} setup.py install --user
          DEPENDEES sitk_create_userbase_step
          WORKING_DIRECTORY <BINARY_DIR>/Wrapping/PythonPackage
        )
      endif()
    endif()

    mitkFunctionInstallExternalCMakeProject(${proj})
    # Still need to install the SimpleITK Python wrappings

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()

