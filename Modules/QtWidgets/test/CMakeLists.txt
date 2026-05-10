MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(QmitkAbstractNodeSelectionWidgetTest QmitkAbstractNodeSelectionWidgetTest
  -platform minimal
)

mitkAddCustomModuleTest(QmitkMxNExplicitNameTest QmitkMxNExplicitNameTest
  -platform minimal
)

mitkAddCustomModuleTest(QmitkMxNLayoutV2Test QmitkMxNLayoutV2Test
  -platform minimal
)

mitkAddCustomModuleTest(QmitkMxNSyncGroupApiTest QmitkMxNSyncGroupApiTest
  -platform minimal
)

mitkAddCustomModuleTest(QmitkSynchronizedWidgetConnectorTest QmitkSynchronizedWidgetConnectorTest
  -platform minimal
)

# Schema lint for the MxN v2 layout. Gated on MITK_USE_Python3 so the test is
# registered only in build configurations that already provide a Python
# interpreter. The lint runs inside a dedicated venv provisioned by a CTest
# fixture (mitkMxNLayoutSchemaLintSetup) - this keeps the host Python
# untouched and lets a missing 'jsonschema' surface as a hard failure on CI
# instead of being silently skipped. When MITK_USE_Python3 is OFF the test is
# not registered at all - the C++ tests load every fixture through
# ApplyLayout regardless, so structural drift is still caught.
if(MITK_USE_Python3 AND BUILD_TESTING)
  set(_mitk_schema_lint_venv "${CMAKE_CURRENT_BINARY_DIR}/MxNLayoutSchemaLintEnv")
  if(WIN32)
    set(_mitk_schema_lint_python "${_mitk_schema_lint_venv}/Scripts/python.exe")
  else()
    set(_mitk_schema_lint_python "${_mitk_schema_lint_venv}/bin/python")
  endif()

  add_test(
    NAME mitkMxNLayoutSchemaLintSetup
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/setup_schema_lint_env.py
            ${_mitk_schema_lint_venv})
  set_tests_properties(mitkMxNLayoutSchemaLintSetup PROPERTIES
    LABELS "MxN;Schema"
    FIXTURES_SETUP MxNLayoutSchemaLintEnv)

  # Test-only fixtures live in MITK-Data under MxNEditor/. The script can
  # validate any schema-valid input it is handed; intentionally schema-
  # invalid fixtures (e.g. the view-direction typo, the v1.x migration-
  # script inputs) are deliberately omitted - they are exercised at the
  # C++ / migration-script layer instead.
  add_test(
    NAME mitkMxNLayoutSchemaLint
    COMMAND ${_mitk_schema_lint_python}
            ${CMAKE_CURRENT_SOURCE_DIR}/validate_layout_fixtures.py
            ${CMAKE_SOURCE_DIR}/Modules/QtWidgets/resource/mxn-layout-v2.schema.json
            # Production resource (shipped with the build):
            ${CMAKE_SOURCE_DIR}/Modules/QtWidgets/resource/mxnLayout_twoRowsEachDirection.json
            # Test-only v2 fixtures from MITK-Data; loader-level rejection
            # cases (strict-mode missing reference, duplicate names) are
            # schema-valid - the v2 schema does not enforce cross-document
            # group resolution or window-name uniqueness; the C++ loader
            # does:
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_lazy_mode.json
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_strict_missing_reference.json
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_duplicate_ids.json
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_seed_widget0_first.json
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_seed_widget1_first.json)
  set_tests_properties(mitkMxNLayoutSchemaLint PROPERTIES
    LABELS "MxN;Schema"
    FIXTURES_REQUIRED MxNLayoutSchemaLintEnv)

  # Behavioural tests for the v1 -> v2 migration script. The script's
  # self-validation covers structural drift; this unittest pins
  # behavioural details (group naming convention, selectAll first-encounter,
  # pre-order widget numbering, sibling-size GCD reduction, root wrapping).
  add_test(
    NAME mitkMxNLayoutMigrationScriptTest
    COMMAND ${Python3_EXECUTABLE} -m unittest
            -v test_migrate_mxn_layout
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_tests_properties(mitkMxNLayoutMigrationScriptTest PROPERTIES
    LABELS "MxN;Migration")
endif()
