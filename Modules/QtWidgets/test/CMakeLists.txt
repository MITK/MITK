MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(QmitkDataStorageTreeModelTest QmitkDataStorageTreeModelTest
  -platform minimal
)

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

# Schema validation of the MxN v2 layout (schema examples, in-tree preset,
# MITK-Data fixtures) is exercised by the pytest suite in
# Modules/Python/test/pytest/test_layout/test_schema_round_trip.py. That
# suite runs cross-platform inside the embedded-Python venv set up by
# mitkPythonBindingsTest, which now also installs 'jsonschema'.

# Behavioural tests for the v1 -> v2 migration script. The script's
# self-validation covers structural drift; this unittest pins
# behavioural details (group naming convention, selectAll first-encounter,
# pre-order widget numbering, sibling-size GCD reduction, root wrapping).
# Stays cross-OS (no Linux gate) because the unittest runs against the
# Python stdlib only and verifies code behaviour, which is worth
# exercising on every supported platform.
if(MITK_USE_Python3 AND BUILD_TESTING)
  add_test(
    NAME mitkMxNLayoutMigrationScriptTest
    COMMAND ${Python3_EXECUTABLE} -m unittest
            -v test_migrate_mxn_layout
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_tests_properties(mitkMxNLayoutMigrationScriptTest PROPERTIES
    LABELS "MxN;Migration")
endif()
