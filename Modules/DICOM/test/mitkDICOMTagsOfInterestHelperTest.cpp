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
  MITK_TEST(PatientPhysicalTags);
  MITK_TEST(AcquisitionInformationTags);
  MITK_TEST(SOPTags);
  MITK_TEST(SourceImageReferenceTags);

  MITK_TEST(FunctionalGroupTagRegistersItsFrameRelativeForm);
  MITK_TEST(BothRootsOfOneAttributeShareOneDerivedRegistration);
  MITK_TEST(RemovingOneRootKeepsTheSiblingsDerivedRegistration);
  MITK_TEST(RemovingOneRootKeepsTheSiblingsPersistenceRequest);
  MITK_TEST(RemovingARootKeepsADirectlyRegisteredFrameRelativeTag);
  MITK_TEST(RemovingADirectlyRegisteredFrameRelativeTagKeepsTheRoots);
  MITK_TEST(RemovingAFunctionalGroupTagLeavesTopLevelTagsAlone);
  MITK_TEST(RemoveAllTagsRemovesBothForms);

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
  std::vector<mitk::DICOMTagPath> m_Registered;

  /** The published key of a functional-group attribute: the path relative to
      the functional-group item, which is what the reader puts on the image. */
  static std::string PublishedName()
  {
    return "DICOM.0028.9145.[0].0028.1053";
  }

  static mitk::DICOMTagPath RootedIn(unsigned int rootElement)
  {
    mitk::DICOMTagPath path;
    path.AddAnySelection(0x5200, rootElement);
    path.AddAnySelection(0x0028, 0x9145);
    path.AddElement(0x0028, 0x1053);
    return path;
  }

  void Register(const mitk::DICOMTagPath& path)
  {
    m_TagsOfInterest->AddTagOfInterest(path);
    m_Registered.push_back(path);
  }

  void Register(const mitk::DICOMTagPath& path, bool makePersistant)
  {
    m_TagsOfInterest->AddTagOfInterest(path, makePersistant);
    m_Registered.push_back(path);
  }

  /** A plain top-level tag, the shape every classic single-frame series uses. */
  static mitk::DICOMTagPath TopLevel(unsigned int group, unsigned int element)
  {
    return mitk::DICOMTagPath(mitk::DICOMTag(group, element));
  }

  bool PublishedKeyIsPersisted() const
  {
    return m_Persistence->HasInfo(PublishedName(), true);
  }

  /** Resolves the two services the derived registration writes through.
      Asserted rather than skipped: a case that quietly returns when a service
      is missing would pass without testing anything, which is the failure mode
      these cases exist to prevent. */
  void ResolveServices()
  {
    auto* context = us::GetModuleContext();

    const auto toiRefs = context->GetServiceReferences<mitk::IDICOMTagsOfInterest>();
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the DICOM tags-of-interest service is registered",
                           !toiRefs.empty());
    const auto persistenceRefs = context->GetServiceReferences<mitk::IPropertyPersistence>();
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the property persistence service is registered",
                           !persistenceRefs.empty());

    m_TagsOfInterest = context->GetService<mitk::IDICOMTagsOfInterest>(toiRefs.front());
    m_Persistence = context->GetService<mitk::IPropertyPersistence>(persistenceRefs.front());

    CPPUNIT_ASSERT_MESSAGE("Test precondition: both services resolve",
                           nullptr != m_TagsOfInterest && nullptr != m_Persistence);
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

  /** A functional-group attribute is scanned under its rooted path but
      published under the frame-relative one. Without the derived registration
      the property matches no persistence info and ItkImageIO drops it on save
      without a log line. */
  void FunctionalGroupTagRegistersItsFrameRelativeForm()
  {
    this->ResolveServices();

    CPPUNIT_ASSERT_MESSAGE("Test precondition: the published key is not registered yet",
                           !this->PublishedKeyIsPersisted());

    this->Register(RootedIn(0x9230));

    CPPUNIT_ASSERT_MESSAGE("Registering the rooted tag persists the published key",
                           this->PublishedKeyIsPersisted());

    // The rooted form stays registered: a file whose per-frame item count does
    // not match its frame count keeps the one-frame model and publishes rooted
    // keys, which must go on being persisted.
    CPPUNIT_ASSERT_MESSAGE("The rooted form stays registered",
                           m_Persistence->HasInfo("DICOM.5200.9230.[0].0028.9145.[0].0028.1053", true));
  }

  void BothRootsOfOneAttributeShareOneDerivedRegistration()
  {
    this->ResolveServices();

    this->Register(RootedIn(0x9230));
    this->Register(RootedIn(0x9229));

    CPPUNIT_ASSERT_MESSAGE("The published key is persisted once both roots are registered",
                           this->PublishedKeyIsPersisted());
    CPPUNIT_ASSERT_MESSAGE("The scan set holds the two rooted paths, not the derived one",
                           !m_TagsOfInterest->HasTag(mitk::FunctionalGroupRelativePath(RootedIn(0x9230))));
  }

  /** The shared and the per-frame root derive to one path, so removing one of
      them must not unregister the other's published key. */
  void RemovingOneRootKeepsTheSiblingsDerivedRegistration()
  {
    this->ResolveServices();

    this->Register(RootedIn(0x9230));
    this->Register(RootedIn(0x9229));
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the published key is persisted",
                           this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveTag(RootedIn(0x9229));

    CPPUNIT_ASSERT_MESSAGE("The surviving root keeps the published key persisted",
                           this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveTag(RootedIn(0x9230));
    m_Registered.clear();

    CPPUNIT_ASSERT_MESSAGE("Removing the last root removes the published key",
                           !this->PublishedKeyIsPersisted());
  }

  /** The repair after a removal must re-register what the survivor asked for,
      not assume persistence. */
  void RemovingOneRootKeepsTheSiblingsPersistenceRequest()
  {
    this->ResolveServices();

    this->Register(RootedIn(0x9230), false);
    this->Register(RootedIn(0x9229), false);
    CPPUNIT_ASSERT_MESSAGE("Test precondition: a non-persistent tag does not persist its published key",
                           !this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveTag(RootedIn(0x9229));

    CPPUNIT_ASSERT_MESSAGE("Removing a sibling must not turn a non-persistent tag persistent",
                           !this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveTag(RootedIn(0x9230));
    m_Registered.clear();
  }

  /** The frame-relative path can be a tag of interest in its own right. Removing
      a rooted tag that happens to derive to it must not take its registration. */
  void RemovingARootKeepsADirectlyRegisteredFrameRelativeTag()
  {
    this->ResolveServices();

    const auto derived = mitk::FunctionalGroupRelativePath(RootedIn(0x9230));

    this->Register(derived);
    this->Register(RootedIn(0x9230));
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the published key is persisted",
                           this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveTag(RootedIn(0x9230));

    CPPUNIT_ASSERT_MESSAGE("A directly registered frame-relative tag survives a rooted sibling's removal",
                           this->PublishedKeyIsPersisted());
    CPPUNIT_ASSERT_MESSAGE("The directly registered tag is still in the scan set",
                           m_TagsOfInterest->HasTag(derived));

    m_TagsOfInterest->RemoveTag(derived);
    m_Registered.clear();

    CPPUNIT_ASSERT_MESSAGE("Removing the last holder removes the published key",
                           !this->PublishedKeyIsPersisted());
  }

  /** The mirror of the case above: the removed path is itself the key a rooted
      tag publishes under, so the rooted tag's registration has to survive. */
  void RemovingADirectlyRegisteredFrameRelativeTagKeepsTheRoots()
  {
    this->ResolveServices();

    const auto derived = mitk::FunctionalGroupRelativePath(RootedIn(0x9230));

    this->Register(derived);
    this->Register(RootedIn(0x9230));
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the published key is persisted",
                           this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveTag(derived);

    CPPUNIT_ASSERT_MESSAGE("The rooted tag still publishes under the key it derives to",
                           this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveTag(RootedIn(0x9230));
    m_Registered.clear();

    CPPUNIT_ASSERT_MESSAGE("Removing the last holder removes the published key",
                           !this->PublishedKeyIsPersisted());
  }

  /** The regression guard for every non-multi-frame input: adding and removing a
      functional-group tag must leave the classic top-level registrations, which
      single-frame series depend on, exactly as they were. */
  void RemovingAFunctionalGroupTagLeavesTopLevelTagsAlone()
  {
    this->ResolveServices();

    // Deliberately not tags from GetDefaultDICOMTagsOfInterest: the service is
    // shared with every other test in the driver, so this test must only ever
    // remove registrations it made itself.
    const auto softwareVersions = TopLevel(0x0018, 0x1020);
    const auto dateOfLastCalibration = TopLevel(0x0018, 0x1200);

    this->Register(softwareVersions);
    this->Register(dateOfLastCalibration);

    const std::string softwareVersionsName = mitk::DICOMTagPathToPropertyName(softwareVersions);
    const std::string dateOfLastCalibrationName = mitk::DICOMTagPathToPropertyName(dateOfLastCalibration);

    CPPUNIT_ASSERT_MESSAGE("Test precondition: the top-level tags are persisted",
                           m_Persistence->HasInfo(softwareVersionsName, true)
                           && m_Persistence->HasInfo(dateOfLastCalibrationName, true));

    this->Register(RootedIn(0x9230));
    this->Register(RootedIn(0x9229));

    CPPUNIT_ASSERT_MESSAGE("Registering functional-group tags leaves the top-level ones persisted",
                           m_Persistence->HasInfo(softwareVersionsName, true)
                           && m_Persistence->HasInfo(dateOfLastCalibrationName, true));

    m_TagsOfInterest->RemoveTag(RootedIn(0x9229));
    m_TagsOfInterest->RemoveTag(RootedIn(0x9230));

    CPPUNIT_ASSERT_MESSAGE("Removing functional-group tags leaves the top-level ones persisted",
                           m_Persistence->HasInfo(softwareVersionsName, true)
                           && m_Persistence->HasInfo(dateOfLastCalibrationName, true));
    CPPUNIT_ASSERT_MESSAGE("Removing functional-group tags leaves the top-level ones in the scan set",
                           m_TagsOfInterest->HasTag(softwareVersions)
                           && m_TagsOfInterest->HasTag(dateOfLastCalibration));

    m_TagsOfInterest->RemoveTag(softwareVersions);
    m_TagsOfInterest->RemoveTag(dateOfLastCalibration);
    m_Registered.clear();
  }

  void RemoveAllTagsRemovesBothForms()
  {
    this->ResolveServices();

    this->Register(RootedIn(0x9230));
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the published key is persisted",
                           this->PublishedKeyIsPersisted());

    m_TagsOfInterest->RemoveAllTags();
    m_Registered.clear();

    CPPUNIT_ASSERT_MESSAGE("RemoveAllTags removes the derived registration too",
                           !this->PublishedKeyIsPersisted());
    CPPUNIT_ASSERT_MESSAGE("RemoveAllTags removes the rooted registration",
                           !m_Persistence->HasInfo("DICOM.5200.9230.[0].0028.9145.[0].0028.1053", true));

    // The suite shares one service with every other test in the driver, so the
    // defaults it just cleared are put back.
    for (const auto& tag : mitk::GetDefaultDICOMTagsOfInterest())
    {
      m_TagsOfInterest->AddTagOfInterest(tag.first);
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMTagsOfInterestHelper)
