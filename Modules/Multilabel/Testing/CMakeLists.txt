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

  # Non-Z-ordered per-frame source keying: permutes the same Z-ordered
  # liver.dcm in-memory, so it shares the fixture gate above. Needs no
  # reference NRRD - it reads the reversed file's own per-frame tags as the
  # oracle.
  mitkAddCustomModuleTest(mitkDICOMSegmentationIONonZOrderedTest_Liver
    mitkDICOMSegmentationIONonZOrderedTest
    ${_dicomSegFixture})
else()
  message(STATUS "MITK DICOM SEG: skipping mitkDICOMSegmentationIORegressionTest_Liver "
    "and mitkDICOMSegmentationIONonZOrderedTest_Liver "
    "(dcmqi fixture not found: ${_dicomSegFixture}).")
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
set(_dicomSegCt3SliceDir
  "${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI/data/segmentations/ct-3slice")
set(_dicomSegCt3SliceList
  "${_dicomSegCt3SliceDir}/01.dcm"
  "${_dicomSegCt3SliceDir}/02.dcm"
  "${_dicomSegCt3SliceDir}/03.dcm")
string(REPLACE ";" "," _dicomSegLabelmapSourceDicoms "${_dicomSegCt3SliceList}")
set(_dicomSegLabelmapReference
  "${MITK_DATA_DIR}/Multilabel/DICOMSegmentationIO/liver-spine-labelmap-dcmqi-reference.nrrd")

set(_dicomSegCt3SliceExists TRUE)
foreach(_path IN LISTS _dicomSegCt3SliceList)
  if(NOT EXISTS "${_path}")
    set(_dicomSegCt3SliceExists FALSE)
  endif()
endforeach()

if(EXISTS "${_dicomSegLabelmapMetadata}"
   AND EXISTS "${_dicomSegLabelmapLiverInput}"
   AND EXISTS "${_dicomSegLabelmapSpineInput}"
   AND EXISTS "${_dicomSegLabelmapReference}"
   AND _dicomSegCt3SliceExists)
  mitkAddCustomModuleTest(mitkDICOMSegmentationIOLabelmapRegressionTest_LiverSpine
    mitkDICOMSegmentationIOLabelmapRegressionTest
    ${_dicomSegLabelmapMetadata}
    ${_dicomSegLabelmapLiverInput}
    ${_dicomSegLabelmapSpineInput}
    ${_dicomSegLabelmapSourceDicoms}
    ${_dicomSegLabelmapReference})
else()
  message(STATUS "MITK DICOM SEG: skipping mitkDICOMSegmentationIOLabelmapRegressionTest_LiverSpine "
    "(dcmqi labelmap fixtures not found under ${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI).")
endif()

# End-to-end coverage for the legacy referenceFiles -> SegSourceImageRelationRule
# migration helper. Uses the same three dcmqi-checked-in source DICOMs as the
# labelmap regression so the test only registers when those files exist.
if(_dicomSegCt3SliceExists)
  mitkAddCustomModuleTest(mitkDICOMSegmentationLegacyMigrationTest
    mitkDICOMSegmentationLegacyMigrationTest
    ${_dicomSegCt3SliceList})
else()
  message(STATUS "MITK DICOM SEG: skipping mitkDICOMSegmentationLegacyMigrationTest "
    "(dcmqi ct-3slice source DICOMs not found under ${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI).")
endif()

# DICOM SEG source references written from a reader-produced source. Shares the
# ct-3slice gate above so an absent dcmqi checkout skips the registration rather
# than the run. The fixture directory reaches the test through a compile
# definition rather than argv: MITK_TEST_SUITE_REGISTRATION drops the driver's
# argv, so a CppUnit fixture has no way to read it.
if(_dicomSegCt3SliceExists)
  mitkAddCustomModuleTest(mitkDICOMSegSourceReferenceTest
    mitkDICOMSegSourceReferenceTest)

  if(TARGET ${TESTDRIVER})
    file(TO_CMAKE_PATH "${_dicomSegCt3SliceDir}" _dicomSegCt3SliceBakedDir)
    target_compile_definitions(${TESTDRIVER} PRIVATE
      MITK_MULTILABEL_DCMQI_CT3SLICE_DIR="${_dicomSegCt3SliceBakedDir}")
  endif()
else()
  message(STATUS "MITK DICOM SEG: skipping mitkDICOMSegSourceReferenceTest "
    "(dcmqi ct-3slice source DICOMs not found under ${MITK_EXTERNAL_PROJECT_PREFIX}/src/DCMQI).")
endif()
