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
    endif()

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    #TODO: Installer and testing works only with static libs on MAC
    set(_build_shared ON)
    if(APPLE)
      set(_build_shared OFF)
    endif()
    ExternalProject_Add(${proj}
       LIST_SEPARATOR ${sep}
       #GIT_REPOSITORY https://github.com/SimpleITK/SimpleITK.git
       # URL https://github.com/SimpleITK/SimpleITK/releases/download/v1.0.1/SimpleITK-1.0.1.tar.xz
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/SimpleITK_9d510bef.tar.gz
       URL_MD5 c34f4d5259594bf1adf1d83f13228cbe
       # PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/SimpleITK-0.8.1.patch
       INSTALL_COMMAND ""
       SOURCE_SUBDIR SuperBuild
       CMAKE_ARGS
         ${ep_common_cache_default_args}
         # -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
      CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
         ${additional_cmake_args}
         -DBUILD_SHARED_LIBS:BOOL=${_build_shared}
         -DSimpleITK_BUILD_DISTRIBUTE:BOOL=ON
         -DSimpleITK_PYTHON_THREADS:BOOL=ON
         -DSimpleITK_USE_SYSTEM_ITK:BOOL=ON
         -DITK_DIR:PATH=${ITK_DIR}
         -DSimpleITK_USE_SYSTEM_SWIG:BOOL=ON
         -DSWIG_DIR:PATH=${SWIG_DIR}
         -DSWIG_EXECUTABLE:FILEPATH=${SWIG_EXECUTABLE}
         -DBUILD_EXAMPLES:BOOL=OFF
         -DBUILD_TESTING:BOOL=OFF
         -DGDCM_DIR:PATH=${GDCM_DIR}
         -DHDF5_DIR:PATH=${HDF5_DIR}
         -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
         -DCMAKE_CXX_COMPILER_AR:FILEPATH=${CMAKE_CXX_COMPILER_AR}
         -DCMAKE_CXX_COMPILER_RANLIB:FILEPATH=${CMAKE_CXX_COMPILER_RANLIB}
         -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
         -DCMAKE_C_COMPILER_AR:FILEPATH=${CMAKE_C_COMPILER_AR}
         -DCMAKE_C_COMPILER_RANLIB:FILEPATH=${CMAKE_C_COMPILER_RANLIB}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    if( MITK_USE_Python )
      set(_sitk_build_dir ${ep_prefix}/src/SimpleITK-build)
      # Build python distribution with easy install. If a own runtime is used
      # embed the egg into the site-package folder of the runtime

      # Note: Userbase install could also be relevant in some cases Probably windows wants to
      # install to Lib/python3.6/
      # Build egg into custom user base folder and deploy it later into installer
      # https://pythonhosted.org/setuptools/easy_install.html#use-the-user-option-and-customize-pythonuserbase
      # PYTHONUSERBASE=${_install_dir} ${PYTHON_EXECUTABLE} setup.py --user

      # PythonDir needs to be fixed for the python interpreter by
      # changing dir delimiter for Windows

      set(_install_dir ${ep_prefix} )
      set(_pythonpath ${ep_prefix}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages )

      if(WIN32)
        STRING(REPLACE "/" "\\\\" _install_dir ${_install_dir})
      else()
        # escape spaces in the install path for linux
        STRING(REPLACE " " "\ " _install_dir ${_install_dir})
      endif()

      ExternalProject_Add_Step(${proj} sitk_python_install_step
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${_pythonpath}$<SEMICOLON>${ep_prefix}/Lib/site-packages ${PYTHON_EXECUTABLE} Packaging/setup.py install --prefix=${_install_dir}
        DEPENDEES install
        WORKING_DIRECTORY ${_sitk_build_dir}/SimpleITK-build/Wrapping/Python/
      )

    endif()

    mitkFunctionInstallExternalCMakeProject(${proj})
    # Still need to install the SimpleITK Python wrappings
  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

  set(SimpleITK_DIR ${ep_prefix}/src/SimpleITK-build/SimpleITK-build)

endif()

