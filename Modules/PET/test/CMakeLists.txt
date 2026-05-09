include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PETIBSIData.cmake)

MITK_CREATE_MODULE_TESTS()

# Tag every test registered by MITK_CREATE_MODULE_TESTS with the "PET"
# label so it shows up grouped in ctest reports and can be selected via
# `ctest -L PET`. files.cmake is the authoritative source list, and the
# macro derives test names from each entry's basename.
include(files.cmake)
foreach(_pet_test ${MODULE_TESTS} ${MODULE_RENDERING_TESTS})
  get_filename_component(_pet_test_name ${_pet_test} NAME_WE)
  if(TEST ${_pet_test_name})
    set_property(TEST ${_pet_test_name} APPEND PROPERTY LABELS "PET")
  endif()
endforeach()

# Bake the resolved IBSI data dir into the test driver as a string
# literal. Compile-time injection keeps a single source of truth: the
# binary always reflects the cache value at build time, whether ctest
# or a developer launches it directly (e.g. via the VS debugger). When
# the cache variable is empty the macro expands to "" and the test
# exits 77 (skip).
if(TARGET MitkPETTestDriver)
  set(_pet_ibsi_data_dir_baked "")
  if(MITK_PET_IBSI_DATA_DIR)
    file(TO_CMAKE_PATH "${MITK_PET_IBSI_DATA_DIR}" _pet_ibsi_data_dir_baked)
  endif()
  target_compile_definitions(MitkPETTestDriver PRIVATE
    MITK_PET_IBSI_DATA_DIR="${_pet_ibsi_data_dir_baked}")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddPETSUVCLISmokeTests.cmake)
