/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMTag.h>
#include <mitkDICOMTagPath.h>
#include <mitkDICOMTagsOfInterestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkDICOMTagsOfInterestHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMTagsOfInterestHelperTestSuite);

  MITK_TEST(PatientModuleTags);
  MITK_TEST(GeneralStudyModuleTags);
  MITK_TEST(ClinicalTrialTags);
  MITK_TEST(GeneralSeriesModuleTags);
  MITK_TEST(VOILUTModuleTags);
  MITK_TEST(ImagePixelModuleTags);
  MITK_TEST(ImagePlaneModuleTags);
  MITK_TEST(SegmentationTags);
  MITK_TEST(RTTags);
  MITK_TEST(RTPlanSequenceTags);
  MITK_TEST(RTStructSequenceTags);
  MITK_TEST(RTDoseSequenceTags);
  MITK_TEST(PETSequenceTags);
  MITK_TEST(PETTopLevelTags);
  MITK_TEST(PatientPhysicalTags);
  MITK_TEST(AcquisitionInformationTags);
  MITK_TEST(SOPTags);
  MITK_TEST(SourceImageReferenceTags);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::DICOMTagPathMapType m_Tags;

  void RequireTopLevel(unsigned int group, unsigned int element, const std::string& label)
  {
    const mitk::DICOMTagPath path = mitk::DICOMTag(group, element);
    CPPUNIT_ASSERT_MESSAGE(
      "Top-level tag missing from default registry: " + label,
      m_Tags.find(path) != m_Tags.end());
  }

  void RequirePath(const mitk::DICOMTagPath& path, const std::string& label)
  {
    CPPUNIT_ASSERT_MESSAGE(
      "Sequence-path tag missing from default registry: " + label,
      m_Tags.find(path) != m_Tags.end());
  }

public:
  void setUp() override
  {
    m_Tags = mitk::GetDefaultDICOMTagsOfInterest();
    CPPUNIT_ASSERT_MESSAGE(
      "GetDefaultDICOMTagsOfInterest() returned empty map",
      !m_Tags.empty());
  }

  void tearDown() override
  {
    m_Tags.clear();
  }

  void PatientModuleTags()
  {
    RequireTopLevel(0x0010, 0x0010, "(0010,0010) PatientsName");
    RequireTopLevel(0x0010, 0x0020, "(0010,0020) PatientID");
    RequireTopLevel(0x0010, 0x0030, "(0010,0030) PatientsBirthDate");
    RequireTopLevel(0x0010, 0x0040, "(0010,0040) PatientsSex");
    RequireTopLevel(0x0010, 0x0032, "(0010,0032) PatientsBirthTime");
    RequireTopLevel(0x0010, 0x1000, "(0010,1000) OtherPatientIDs");
    RequireTopLevel(0x0010, 0x1001, "(0010,1001) OtherPatientNames");
    RequireTopLevel(0x0010, 0x2160, "(0010,2160) EthnicGroup");
    RequireTopLevel(0x0010, 0x4000, "(0010,4000) PatientComments");
    RequireTopLevel(0x0012, 0x0062, "(0012,0062) PatientIdentityRemoved");
    RequireTopLevel(0x0012, 0x0063, "(0012,0063) DeIdentificationMethod");
  }

  void GeneralStudyModuleTags()
  {
    RequireTopLevel(0x0020, 0x000d, "(0020,000d) StudyInstanceUID");
    RequireTopLevel(0x0008, 0x0020, "(0008,0020) StudyDate");
    RequireTopLevel(0x0008, 0x0030, "(0008,0030) StudyTime");
    RequireTopLevel(0x0008, 0x0090, "(0008,0090) ReferringPhysiciansName");
    RequireTopLevel(0x0020, 0x0010, "(0020,0010) StudyID");
    RequireTopLevel(0x0008, 0x0050, "(0008,0050) AccessionNumber");
    RequireTopLevel(0x0008, 0x1030, "(0008,1030) StudyDescription");
    RequireTopLevel(0x0008, 0x1048, "(0008,1048) PhysiciansOfRecord");
    RequireTopLevel(0x0008, 0x1060, "(0008,1060) NameOfPhysicianReadingStudy");
  }

  void ClinicalTrialTags()
  {
    RequireTopLevel(0x0012, 0x0010, "(0012,0010) ClinicalTrialSponsorName");
    RequireTopLevel(0x0012, 0x0020, "(0012,0020) ClinicalTrialProtocolID");
    RequireTopLevel(0x0012, 0x0021, "(0012,0021) ClinicalTrialProtocolName");
    RequireTopLevel(0x0012, 0x0030, "(0012,0030) ClinicalTrialSiteID");
    RequireTopLevel(0x0012, 0x0031, "(0012,0031) ClinicalTrialSiteName");
    RequireTopLevel(0x0012, 0x0040, "(0012,0040) ClinicalTrialSubjectID");
    RequireTopLevel(0x0012, 0x0042, "(0012,0042) ClinicalTrialSubjectReadingID");
    RequireTopLevel(0x0012, 0x0050, "(0012,0050) ClinicalTrialTimePointID");
    RequireTopLevel(0x0012, 0x0060, "(0012,0060) ClinicalTrialCoordinatingCenterName");
    RequireTopLevel(0x0012, 0x0071, "(0012,0071) ClinicalTrialSeriesID");
  }

  void GeneralSeriesModuleTags()
  {
    RequireTopLevel(0x0008, 0x0060, "(0008,0060) Modality");
    RequireTopLevel(0x0020, 0x000e, "(0020,000e) SeriesInstanceUID");
    RequireTopLevel(0x0020, 0x0011, "(0020,0011) SeriesNumber");
    RequireTopLevel(0x0020, 0x0060, "(0020,0060) Laterality");
    RequireTopLevel(0x0008, 0x0021, "(0008,0021) SeriesDate");
    RequireTopLevel(0x0008, 0x0031, "(0008,0031) SeriesTime");
    RequireTopLevel(0x0008, 0x1050, "(0008,1050) PerformingPhysiciansName");
    RequireTopLevel(0x0018, 0x1030, "(0018,1030) ProtocolName");
    RequireTopLevel(0x0008, 0x103e, "(0008,103e) SeriesDescription");
    RequireTopLevel(0x0008, 0x1070, "(0008,1070) OperatorsName");
    RequireTopLevel(0x0018, 0x0015, "(0018,0015) BodyPartExamined");
    RequireTopLevel(0x0018, 0x5100, "(0018,5100) PatientPosition");
    RequireTopLevel(0x0028, 0x0108, "(0028,0108) SmallestPixelValueInSeries");
    RequireTopLevel(0x0028, 0x0109, "(0028,0109) LargestPixelValueInSeries");
  }

  void VOILUTModuleTags()
  {
    RequireTopLevel(0x0028, 0x1050, "(0028,1050) WindowCenter");
    RequireTopLevel(0x0028, 0x1051, "(0028,1051) WindowWidth");
    RequireTopLevel(0x0028, 0x1055, "(0028,1055) WindowCenterAndWidthExplanation");
  }

  void ImagePixelModuleTags()
  {
    RequireTopLevel(0x0028, 0x0004, "(0028,0004) PhotometricInterpretation");
    RequireTopLevel(0x0028, 0x0010, "(0028,0010) Rows");
    RequireTopLevel(0x0028, 0x0011, "(0028,0011) Columns");
  }

  void ImagePlaneModuleTags()
  {
    RequireTopLevel(0x0028, 0x0030, "(0028,0030) PixelSpacing");
    RequireTopLevel(0x0018, 0x1164, "(0018,1164) ImagerPixelSpacing");
  }

  void SegmentationTags()
  {
    RequireTopLevel(0x0070, 0x0080, "(0070,0080) ContentLabel");
    RequireTopLevel(0x0070, 0x0081, "(0070,0081) ContentDescription");
    RequireTopLevel(0x0070, 0x0084, "(0070,0084) ContentCreatorName");
  }

  void RTTags()
  {
    RequireTopLevel(0x0028, 0x1052, "(0028,1052) RescaleIntercept");
    RequireTopLevel(0x0028, 0x1053, "(0028,1053) RescaleSlope");
    RequireTopLevel(0x0008, 0x1090, "(0008,1090) ManufacturerModelName");
    RequireTopLevel(0x0008, 0x0070, "(0008,0070) ManufacturerName");
    RequireTopLevel(0x0008, 0x0080, "(0008,0080) InstitutionName");
    RequireTopLevel(0x0008, 0x1010, "(0008,1010) StationName");
    RequireTopLevel(0x3004, 0x000e, "(3004,000e) DoseGridScaling");
  }

  void RTPlanSequenceTags()
  {
    mitk::DICOMTagPath doseRefSeq;
    doseRefSeq.AddAnySelection(0x300A, 0x0010);
    mitk::DICOMTagPath fractionGroupSeq;
    fractionGroupSeq.AddAnySelection(0x300A, 0x0070);
    mitk::DICOMTagPath beamSeq;
    beamSeq.AddAnySelection(0x300A, 0x00B0);
    mitk::DICOMTagPath refStructSetSeq;
    refStructSetSeq.AddAnySelection(0x300C, 0x0060);

    RequirePath(mitk::DICOMTagPath(doseRefSeq).AddElement(0x300A, 0x0013),
                "DoseReferenceSequence/(300A,0013) DoseReferenceUID");
    RequirePath(mitk::DICOMTagPath(doseRefSeq).AddElement(0x300A, 0x0016),
                "DoseReferenceSequence/(300A,0016) DoseReferenceDescription");
    RequirePath(mitk::DICOMTagPath(doseRefSeq).AddElement(0x300A, 0x0026),
                "DoseReferenceSequence/(300A,0026) TargetPrescriptionDose");
    RequirePath(mitk::DICOMTagPath(fractionGroupSeq).AddElement(0x300A, 0x0078),
                "FractionGroupSequence/(300A,0078) NumberOfFractionsPlanned");
    RequirePath(mitk::DICOMTagPath(fractionGroupSeq).AddElement(0x300A, 0x0080),
                "FractionGroupSequence/(300A,0080) NumberOfBeams");
    RequirePath(mitk::DICOMTagPath(beamSeq).AddElement(0x300A, 0x00C6),
                "BeamSequence/(300A,00C6) RadiationType");
    RequirePath(mitk::DICOMTagPath(refStructSetSeq).AddElement(0x0008, 0x1155),
                "ReferencedStructureSetSequence/(0008,1155) ReferencedSOPInstanceUID");
  }

  void RTStructSequenceTags()
  {
    mitk::DICOMTagPath structSetROISeq;
    structSetROISeq.AddAnySelection(0x3006, 0x0020);

    RequirePath(mitk::DICOMTagPath(structSetROISeq).AddElement(0x3006, 0x0022),
                "StructureSetROISequence/(3006,0022) ROINumber");
    RequirePath(mitk::DICOMTagPath(structSetROISeq).AddElement(0x3006, 0x0026),
                "StructureSetROISequence/(3006,0026) ROIName");
    RequirePath(mitk::DICOMTagPath(structSetROISeq).AddElement(0x3006, 0x0024),
                "StructureSetROISequence/(3006,0024) ReferencedFrameOfReferenceUID");
  }

  void RTDoseSequenceTags()
  {
    mitk::DICOMTagPath planRefSeq;
    planRefSeq.AddAnySelection(0x300C, 0x0002);

    RequirePath(mitk::DICOMTagPath(planRefSeq).AddElement(0x0008, 0x1155),
                "PlanReferenceSequence/(0008,1155) ReferencedSOPInstanceUID");
  }

  void PETSequenceTags()
  {
    mitk::DICOMTagPath radioPharmaRoot;
    radioPharmaRoot.AddAnySelection(0x0054, 0x0016);
    mitk::DICOMTagPath radioNuclideRoot(radioPharmaRoot);
    radioNuclideRoot.AddAnySelection(0x0054, 0x0300);

    RequirePath(mitk::DICOMTagPath(radioPharmaRoot).AddElement(0x0018, 0x0031),
                "RadiopharmaceuticalInformationSequence/(0018,0031) Radiopharmaceutical");
    RequirePath(mitk::DICOMTagPath(radioPharmaRoot).AddElement(0x0018, 0x1072),
                "RadiopharmaceuticalInformationSequence/(0018,1072) RadiopharmaceuticalStartTime");
    RequirePath(mitk::DICOMTagPath(radioPharmaRoot).AddElement(0x0018, 0x1078),
                "RadiopharmaceuticalInformationSequence/(0018,1078) RadiopharmaceuticalStartDateTime");
    RequirePath(mitk::DICOMTagPath(radioPharmaRoot).AddElement(0x0018, 0x1074),
                "RadiopharmaceuticalInformationSequence/(0018,1074) RadionuclideTotalDose");
    RequirePath(mitk::DICOMTagPath(radioPharmaRoot).AddElement(0x0018, 0x1075),
                "RadiopharmaceuticalInformationSequence/(0018,1075) RadionuclideHalfLife");
    RequirePath(mitk::DICOMTagPath(radioPharmaRoot).AddElement(0x0018, 0x1076),
                "RadiopharmaceuticalInformationSequence/(0018,1076) RadionuclidePositronFraction");

    RequirePath(mitk::DICOMTagPath(radioNuclideRoot).AddElement(0x0008, 0x0100),
                "RadionuclideCodeSequence/(0008,0100) CodeValue");
    RequirePath(mitk::DICOMTagPath(radioNuclideRoot).AddElement(0x0008, 0x0102),
                "RadionuclideCodeSequence/(0008,0102) CodingSchemeDesignator");
    RequirePath(mitk::DICOMTagPath(radioNuclideRoot).AddElement(0x0008, 0x0104),
                "RadionuclideCodeSequence/(0008,0104) CodeMeaning");
  }

  void PETTopLevelTags()
  {
    RequireTopLevel(0x0054, 0x1001, "(0054,1001) RadioactivityUnits");
    RequireTopLevel(0x0054, 0x1006, "(0054,1006) SUVType");
    RequireTopLevel(0x0054, 0x1102, "(0054,1102) DecayCorrection");
    RequireTopLevel(0x0054, 0x1321, "(0054,1321) DecayFactor");
    RequireTopLevel(0x0054, 0x1300, "(0054,1300) FrameReferenceTime");
    RequireTopLevel(0x0018, 0x1242, "(0018,1242) ActualFrameDuration");
  }

  void PatientPhysicalTags()
  {
    RequireTopLevel(0x0010, 0x1030, "(0010,1030) PatientWeight");
    RequireTopLevel(0x0010, 0x1020, "(0010,1020) PatientSize");
  }

  void AcquisitionInformationTags()
  {
    RequireTopLevel(0x0008, 0x0022, "(0008,0022) AcquisitionDate");
    RequireTopLevel(0x0008, 0x0032, "(0008,0032) AcquisitionTime");
    RequireTopLevel(0x0008, 0x002a, "(0008,002a) AcquisitionDateTime");
    RequireTopLevel(0x0018, 0x002a, "(0018,002a) SequenceName");
    RequireTopLevel(0x0018, 0x0020, "(0018,0020) ScanningSequence");
    RequireTopLevel(0x0018, 0x0021, "(0018,0021) SequenceVariant");
    RequireTopLevel(0x0018, 0x0080, "(0018,0080) RepetitionTime");
    RequireTopLevel(0x0018, 0x0081, "(0018,0081) EchoTime");
    RequireTopLevel(0x0018, 0x1310, "(0018,1310) AcquisitionMatrix");
    RequireTopLevel(0x0018, 0x0087, "(0018,0087) MagneticFieldStrength");
  }

  void SOPTags()
  {
    RequireTopLevel(0x0008, 0x0018, "(0008,0018) SOPInstanceUID");
    RequireTopLevel(0x0020, 0x0013, "(0020,0013) InstanceNumber");
    RequireTopLevel(0x0020, 0x1041, "(0020,1041) SliceLocation");
  }

  void SourceImageReferenceTags()
  {
    mitk::DICOMTagPath sourceImageRefRoot;
    sourceImageRefRoot.AddAnySelection(0x0008, 0x2112);
    mitk::DICOMTagPath sourceImageRefPurposeRoot(sourceImageRefRoot);
    sourceImageRefPurposeRoot.AddAnySelection(0x0040, 0xa170);

    RequirePath(mitk::DICOMTagPath(sourceImageRefRoot).AddElement(0x0008, 0x1155),
                "SourceImageSequence/(0008,1155) ReferencedSOPInstanceUID");
    RequirePath(mitk::DICOMTagPath(sourceImageRefRoot).AddElement(0x0008, 0x1150),
                "SourceImageSequence/(0008,1150) ReferencedSOPClassUID");

    RequirePath(mitk::DICOMTagPath(sourceImageRefPurposeRoot).AddElement(0x0008, 0x0104),
                "SourceImage Purpose/(0008,0104) CodeMeaning");
    RequirePath(mitk::DICOMTagPath(sourceImageRefPurposeRoot).AddElement(0x0008, 0x0100),
                "SourceImage Purpose/(0008,0100) CodeValue");
    RequirePath(mitk::DICOMTagPath(sourceImageRefPurposeRoot).AddElement(0x0008, 0x0102),
                "SourceImage Purpose/(0008,0102) CodeSchemeDesignator");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMTagsOfInterestHelper)
