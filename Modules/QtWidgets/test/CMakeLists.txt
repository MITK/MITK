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

# Optional schema lint for the MxN v2 layout. Gated on MITK_USE_Python3 so the
# test is registered only in build configurations that already provide a Python
# interpreter; when 'jsonschema' is missing in that interpreter, the script
# exits 77 and CTest reports the test as skipped (no false failure). When
# MITK_USE_Python3 is OFF the test is not registered at all - the C++ tests
# load every fixture through ApplyLayout regardless, so structural drift is
# caught even when the lint is skipped.
if(MITK_USE_Python3 AND BUILD_TESTING)
  # Test-only fixtures live in MITK-Data under MxNEditor/. The script can
  # validate any schema-valid input it is handed; intentionally schema-
  # invalid fixtures (e.g. the view-direction typo, the v1.x migration-
  # script inputs) are deliberately omitted - they are exercised at the
  # C++ / migration-script layer instead.
  add_test(
    NAME mitkMxNLayoutSchemaLint
    COMMAND ${Python3_EXECUTABLE}
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
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_duplicate_names.json
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_seed_widget0_first.json
            ${MITK_DATA_DIR}/MxNEditor/mxn_v2_seed_widget1_first.json)
  set_tests_properties(mitkMxNLayoutSchemaLint PROPERTIES
    LABELS "MxN;Schema"
    SKIP_RETURN_CODE 77)
endif()
