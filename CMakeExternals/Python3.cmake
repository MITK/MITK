#------------------------------------------------------------
# Python Standalone Builds
#------------------------------------------------------------

include("${CMAKE_CURRENT_LIST_DIR}/Python3/mitkFunctionAddStandalonePython.cmake")

if(MITK_USE_Python3)
  set(proj Python3)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  # Sanity checks
  if(DEFINED ${proj}_DIR AND NOT EXISTS "${${proj}_DIR}")
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    #[[ Patch versions and archive hashes per CPython series. Regenerate on a
        release bump with
        Python3/generate_versions.py <release> Python3/Versions.cmake ]]
    include("${CMAKE_CURRENT_LIST_DIR}/Python3/Versions.cmake")

    #[[ The CPython embedded in MITK. Deliberately lags the newest release: it
        has to run the AI segmentation tools, whose PyTorch dependency reaches a
        new CPython series later than the wheel-building infrastructure does.
        Independent of the CPython versions the mitk Python wheel targets. ]]
    set(MITK_Python3_VERSION "3.12" CACHE STRING "CPython version embedded in MITK")
    set_property(CACHE MITK_Python3_VERSION PROPERTY STRINGS ${MITK_PYTHON3_VERSIONS})
    mark_as_advanced(MITK_Python3_VERSION)

    if(NOT MITK_Python3_VERSION IN_LIST MITK_PYTHON3_VERSIONS)
      message(FATAL_ERROR "MITK_Python3_VERSION is \"${MITK_Python3_VERSION}\", but release "
        "${MITK_PYTHON3_RELEASE} provides these CPython versions: ${MITK_PYTHON3_VERSIONS}")
    endif()

    set(install_dir "${MITK_BINARY_DIR}/MITK-build/python")

    mitkFunctionAddStandalonePython(${proj} ${MITK_Python3_VERSION} "${install_dir}"
      WITH_NUMPY
      CHECK_OPENSSL
    )

    set(Python3_DIR ${install_dir})
    set(Python3_ROOT_DIR ${install_dir})

    #[[ CPython versions the mitk Python wheel is built for. pybind11 has no
        stable-ABI support, so one extension module per CPython series is
        unavoidable. Only that one target depends on the series, so the
        additional interpreters are staged for their headers and, on Windows,
        their import library. They are neither provisioned nor shipped, which is
        why they get neither of the function's options.

        Acted upon only by the PythonWheel configuration. Every other
        configuration builds no wheels, and downloading interpreters it would
        never compile against would be pure cost. ]]
    set(MITK_Python3_WHEEL_VERSIONS "${MITK_PYTHON3_VERSIONS}" CACHE STRING
      "CPython versions to build mitk Python wheels for")
    mark_as_advanced(MITK_Python3_WHEEL_VERSIONS)

    if(MITK_BUILD_CONFIGURATION STREQUAL "PythonWheel")
      foreach(wheel_version IN LISTS MITK_Python3_WHEEL_VERSIONS)
        if(NOT wheel_version IN_LIST MITK_PYTHON3_VERSIONS)
          message(FATAL_ERROR "MITK_Python3_WHEEL_VERSIONS contains \"${wheel_version}\", but release "
            "${MITK_PYTHON3_RELEASE} provides these CPython versions: ${MITK_PYTHON3_VERSIONS}")
        endif()
      endforeach()

      set(extra_wheel_versions ${MITK_Python3_WHEEL_VERSIONS})
      list(REMOVE_ITEM extra_wheel_versions ${MITK_Python3_VERSION})

      if(extra_wheel_versions AND NOT MITK_USE_pybind11)
        message(FATAL_ERROR "MITK_Python3_WHEEL_VERSIONS asks for wheels beyond the embedded CPython "
          "${MITK_Python3_VERSION}, but building Python wheels requires MITK_USE_pybind11")
      endif()

      foreach(wheel_version IN LISTS extra_wheel_versions)
        set(wheel_proj "Python3-${wheel_version}")

        mitkFunctionAddStandalonePython(${wheel_proj} ${wheel_version}
          "${MITK_BINARY_DIR}/MITK-build/python-${wheel_version}")

        list(APPEND ${proj}_DEPENDS ${wheel_proj})
      endforeach()
    endif()
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
