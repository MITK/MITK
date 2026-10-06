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
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
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
  MITK_TEST(PETFunctionalGroupMacroTags);
  MITK_TEST(PatientPhysicalTags);
  MITK_TEST(AcquisitionInformationTags);
  MITK_TEST(SOPTags);
  MITK_TEST(SourceImageReferenceTags);

  MITK_TEST(TagOfInterestRegistersOnlyItsOwnPath);

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

  mitk::IDICOMTagsOfInterest* m_TagsOfInterest = nullptr;
  mitk::IPropertyPersistence* m_Persistence = nullptr;
  mitk::IPropertyDescriptions* m_Descriptions = nullptr;
  std::vector<mitk::DICOMTagPath> m_Registered;

  void Register(const mitk::DICOMTagPath& path)
  {
    m_TagsOfInterest->AddTagOfInterest(path);
    m_Registered.push_back(path);
  }

  /** Asserted rather than skipped: a case that quietly returns when a service
      is missing would pass without testing anything. */
  void ResolveServices()
  {
    auto* context = us::GetModuleContext();

    const auto toiRefs = context->GetServiceReferences<mitk::IDICOMTagsOfInterest>();
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the DICOM tags-of-interest service is registered",
                           !toiRefs.empty());
    const auto persistenceRefs = context->GetServiceReferences<mitk::IPropertyPersistence>();
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the property persistence service is registered",
                           !persistenceRefs.empty());
    const auto descriptionRefs = context->GetServiceReferences<mitk::IPropertyDescriptions>();
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the property descriptions service is registered",
                           !descriptionRefs.empty());

    m_TagsOfInterest = context->GetService<mitk::IDICOMTagsOfInterest>(toiRefs.front());
    m_Persistence = context->GetService<mitk::IPropertyPersistence>(persistenceRefs.front());
    m_Descriptions = context->GetService<mitk::IPropertyDescriptions>(descriptionRefs.front());

    CPPUNIT_ASSERT_MESSAGE("Test precondition: all three services resolve",
                           nullptr != m_TagsOfInterest && nullptr != m_Persistence && nullptr != m_Descriptions);
  }

  void ReleaseRegistrations()
  {
    if (nullptr != m_TagsOfInterest)
    {
      for (const auto& path : m_Registered)
      {
        m_TagsOfInterest->RemoveTag(path);
      }
    }
    m_Registered.clear();
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
    this->ReleaseRegistrations();
    m_TagsOfInterest = nullptr;
    m_Persistence = nullptr;
    m_Descriptions = nullptr;
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

  /** The Enhanced PET attributes are registered relative to the
      functional-group item, never rooted in (5200,9229) or (5200,9230): the
      reader searches such a path in both groups from the one registration,
      and a rooted one publishes nothing for a frame-model file. */
  void PETFunctionalGroupMacroTags()
  {
    RequireTopLevel(0x0018, 0x9758, "(0018,9758) DecayCorrected");
    RequireTopLevel(0x0018, 0x9701, "(0018,9701) DecayCorrectionDateTime");
    RequireTopLevel(0x0028, 0x0008, "(0028,0008) NumberOfFrames");

    mitk::DICOMTagPath mapping;
    mapping.AddAnySelection(0x0040, 0x9096);
    mitk::DICOMTagPath unitsCode(mapping);
    unitsCode.AddAnySelection(0x0040, 0x08EA);
    RequirePath(mitk::DICOMTagPath(unitsCode).AddElement(0x0008, 0x0100),
                "RealWorldValueMapping/MeasurementUnitsCode/(0008,0100) CodeValue");
    RequirePath(mitk::DICOMTagPath(unitsCode).AddElement(0x0008, 0x0119),
                "RealWorldValueMapping/MeasurementUnitsCode/(0008,0119) LongCodeValue");
    RequirePath(mitk::DICOMTagPath(unitsCode).AddElement(0x0008, 0x0120),
                "RealWorldValueMapping/MeasurementUnitsCode/(0008,0120) URNCodeValue");
    RequirePath(mitk::DICOMTagPath(unitsCode).AddElement(0x0008, 0x0102),
                "RealWorldValueMapping/MeasurementUnitsCode/(0008,0102) CodingSchemeDesignator");
    RequirePath(mitk::DICOMTagPath(mapping).AddElement(0x0040, 0x9225),
                "RealWorldValueMapping/(0040,9225) RealWorldValueSlope");
    RequirePath(mitk::DICOMTagPath(mapping).AddElement(0x0040, 0x9224),
                "RealWorldValueMapping/(0040,9224) RealWorldValueIntercept");

    mitk::DICOMTagPath transformation;
    transformation.AddAnySelection(0x0028, 0x9145);
    RequirePath(mitk::DICOMTagPath(transformation).AddElement(0x0028, 0x1053),
                "PixelValueTransformation/(0028,1053) RescaleSlope");
    RequirePath(mitk::DICOMTagPath(transformation).AddElement(0x0028, 0x1052),
                "PixelValueTransformation/(0028,1052) RescaleIntercept");
    RequirePath(mitk::DICOMTagPath(transformation).AddElement(0x0028, 0x1054),
                "PixelValueTransformation/(0028,1054) RescaleType");

    mitk::DICOMTagPath frameContent;
    frameContent.AddAnySelection(0x0020, 0x9111);
    RequirePath(mitk::DICOMTagPath(frameContent).AddElement(0x0018, 0x9151),
                "FrameContent/(0018,9151) FrameReferenceDateTime");
    RequirePath(mitk::DICOMTagPath(frameContent).AddElement(0x0018, 0x9074),
                "FrameContent/(0018,9074) FrameAcquisitionDateTime");
    RequirePath(mitk::DICOMTagPath(frameContent).AddElement(0x0018, 0x9220),
                "FrameContent/(0018,9220) FrameAcquisitionDuration");

    for (const auto& entry : m_Tags)
    {
      CPPUNIT_ASSERT_MESSAGE("No default tag of interest is rooted in a functional group: " + entry.first.ToStr(),
                             !mitk::IsFunctionalGroupRooted(entry.first));
    }
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
    RequireTopLevel(0x0008, 0x0016, "(0008,0016) SOPClassUID");
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

  /**
   * The service registers exactly the path it is given, whatever its shape.
   *
   * A functional-group-rooted path is the input that discriminates: it is the
   * one shape for which a second registration under the frame-relative key
   * could look useful, and that key must stay unregistered. The reader, not the
   * service, decides where in a file an attribute is searched.
   */
  void TagOfInterestRegistersOnlyItsOwnPath()
  {
    this->ResolveServices();

    // In-Stack Position Number: a Frame Content attribute no default
    // registration covers, so the frame-relative key is provably free.
    const std::string ownName = "DICOM.5200.9230.[0].0020.9111.[0].0020.9057";
    const std::string frameRelativeName = "DICOM.0020.9111.[0].0020.9057";

    CPPUNIT_ASSERT_MESSAGE("Test precondition: the frame-relative key is not registered yet",
                           !m_Persistence->HasInfo(frameRelativeName, true)
                             && !m_Descriptions->HasDescription(frameRelativeName));

    mitk::DICOMTagPath rooted;
    rooted.AddAnySelection(0x5200, 0x9230).AddAnySelection(0x0020, 0x9111).AddElement(0x0020, 0x9057);
    this->Register(rooted);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("One persistence info for the registered path",
                                 std::size_t(1), m_Persistence->GetInfo(ownName, true).size());
    CPPUNIT_ASSERT_MESSAGE("A description for the registered path",
                           m_Descriptions->HasDescription(ownName));

    CPPUNIT_ASSERT_MESSAGE("No persistence info under a derived key",
                           !m_Persistence->HasInfo(frameRelativeName, true));
    CPPUNIT_ASSERT_MESSAGE("No description under a derived key",
                           !m_Descriptions->HasDescription(frameRelativeName));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMTagsOfInterestHelper)
