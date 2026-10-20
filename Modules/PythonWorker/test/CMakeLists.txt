MITK_CREATE_MODULE_TESTS(EXTRA_DEPENDS MitkPythonHelper)

if(TARGET ${TESTDRIVER})
  #[[ The host must be created after MITK_CREATE_MODULE_TESTS: creating it
      overwrites MODULE_NAME, from which the test driver name is derived. ]]
  mitk_create_executable(PythonWorkerTestHost
    DEPENDS MitkPythonWorker
    CPP_FILES MitkPythonWorkerTestHost.cpp
    NO_FEATURE_INFO
    NO_BATCH_FILE
    NO_INSTALL
  )

  add_dependencies(${TESTDRIVER} ${EXECUTABLE_TARGET})
  set_property(TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Modules/Tests")

  get_filename_component(MITK_PYTHON_WORKER_RUNTIME "${CMAKE_CURRENT_SOURCE_DIR}/../resource/mitk_worker.py" ABSOLUTE)

  target_compile_definitions(${TESTDRIVER} PRIVATE
    MITK_PYTHON_WORKER_TEST_MODULE="${CMAKE_CURRENT_SOURCE_DIR}/mitk_worker_test.py"
    MITK_PYTHON_WORKER_RUNTIME="${MITK_PYTHON_WORKER_RUNTIME}"
    MITK_PYTHON_WORKER_TEST_HOST="$<TARGET_FILE:${EXECUTABLE_TARGET}>")

  set_property(TEST mitkPythonWorkerTest PROPERTY TIMEOUT 120)
endif()
