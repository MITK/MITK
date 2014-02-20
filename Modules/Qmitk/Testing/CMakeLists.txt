set(QT_USE_QTTEST 1)
include(${QT_USE_FILE})

MITK_CREATE_MODULE_TESTS(LABELS Qmitk)

mitkAddCustomModuleTest(Qmitk-StdMultiWidgetTest QmitkStdMultiWidgetTest ${NIFTK_DATA_DIR}/Input/volunteers/16856/16856-002-1.img)

#target_link_libraries(QmitkStdMultiWidgetTestDriver ${QT_QTTEST_LIBRARY})
