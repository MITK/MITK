MITK_CREATE_MODULE_TESTS(PACKAGE_DEPENDS DCMQI DCMTK|dcmseg+dcmsr)

# The DICOM SEG regression tests pull their input straight out of dcmqi's
# checked-in test data so the fixtures' origin is explicit. In a SuperBuild,
# dcmqi is cloned under ${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI; in a
# non-SuperBuild build the source tree may not be present, in which case the
# fixture paths do not exist and we skip the registrations.
set(_dicomSegFixture
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/liver.dcm")

if(EXISTS "${_dicomSegFixture}")
  mitkAddCustomModuleTest(mitkDICOMSegmentationIORegressionTest_Liver
    mitkDICOMSegmentationIORegressionTest
    ${_dicomSegFixture}
    ${MITK_DATA_DIR}/Multilabel/DICOMSegmentationIO/liver-dcmqi-reference.nrrd)
endif()

# Labelmap (Sup 243) regression test. The labelmap-format DICOM SEG is built
# from dcmqi's existing inputs at test runtime via the dcmqi
# Itk2DicomConverter (--segmentationType labelmap equivalent), then read back
# through MITK and compared to the checked-in reference segmentation.
set(_dicomSegLabelmapMetadata
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/doc/examples/seg-example_liver_spine.json")
set(_dicomSegLabelmapLiverInput
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/liver_seg.nrrd")
set(_dicomSegLabelmapSpineInput
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/spine_seg.nrrd")
set(_dicomSegLabelmapSourceDicomList
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/ct-3slice/01.dcm"
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/ct-3slice/02.dcm"
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/ct-3slice/03.dcm")
string(REPLACE ";" "," _dicomSegLabelmapSourceDicoms "${_dicomSegLabelmapSourceDicomList}")
set(_dicomSegLabelmapReference
  "${MITK_DATA_DIR}/Multilabel/DICOMSegmentationIO/liver-spine-labelmap-dcmqi-reference.nrrd")

set(_dicomSegLabelmapAllInputsExist TRUE)
foreach(_path IN LISTS _dicomSegLabelmapSourceDicomList)
  if(NOT EXISTS "${_path}")
    set(_dicomSegLabelmapAllInputsExist FALSE)
  endif()
endforeach()

if(EXISTS "${_dicomSegLabelmapMetadata}"
   AND EXISTS "${_dicomSegLabelmapLiverInput}"
   AND EXISTS "${_dicomSegLabelmapSpineInput}"
   AND EXISTS "${_dicomSegLabelmapReference}"
   AND _dicomSegLabelmapAllInputsExist)
  mitkAddCustomModuleTest(mitkDICOMSegmentationIOLabelmapRegressionTest_LiverSpine
    mitkDICOMSegmentationIOLabelmapRegressionTest
    ${_dicomSegLabelmapMetadata}
    ${_dicomSegLabelmapLiverInput}
    ${_dicomSegLabelmapSpineInput}
    ${_dicomSegLabelmapSourceDicoms}
    ${_dicomSegLabelmapReference})
endif()
