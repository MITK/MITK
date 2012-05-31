MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(mitkFiberBundleXReaderWriterTest mitkFiberBundleXReaderWriterTest ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)
## deactivated, see bug 12017
# mitkAddCustomModuleTest(mitkFiberBundleXTest mitkFiberBundleXTest ${MITK_DATA_DIR}/DiffusionImaging/fiberBundleX.fib)
