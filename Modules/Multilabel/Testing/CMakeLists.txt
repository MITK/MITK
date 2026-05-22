MITK_CREATE_MODULE_TESTS()

# The DICOM SEG regression test loads its binary input straight out of dcmqi's
# checked-in test data so the fixture's origin is explicit. In a SuperBuild,
# dcmqi is cloned under ${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI; in a
# non-SuperBuild build the source tree may not be present, in which case the
# fixture path does not exist and we skip the registration.
set(_dicomSegFixture
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/liver.dcm")

if(EXISTS "${_dicomSegFixture}")
  mitkAddCustomModuleTest(mitkDICOMSegmentationIORegressionTest_Liver
    mitkDICOMSegmentationIORegressionTest
    ${_dicomSegFixture}
    ${MITK_DATA_DIR}/Multilabel/DICOMSegmentationIO/liver-dcmqi-reference.nrrd)
endif()
