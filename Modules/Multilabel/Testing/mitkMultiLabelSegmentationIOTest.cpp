/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

#include <mitkArbitraryTimeGeometry.h>
#include <mitkCoreServices.h>
#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkIOMetaInformationPropertyConstants.h>
#include <mitkIPropertyPersistence.h>
#include <mitkImageWriteAccessor.h>
#include <mitkPropertyNameHelper.h>
#include <mitkPropertyPersistenceInfo.h>
#include <mitkSegIOTestCapabilities.h>
#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkSegTestSourceImageFactory.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <filesystem>

class mitkMultiLabelSegmentationIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSegmentationIOTestSuite);
  MITK_TEST(TestReadEmptyMultiLabelSegmentation);
  MITK_TEST(TestReadEmptyMultiLabelSegmentation_withNoMetaInformation);
  MITK_TEST(TestReadEmptyMultiLabelSegmentation_withNoMetaInformation_butContent);
  MITK_TEST(TestDicomDerivedSegSurvivesNativeNrrdRoundTrip);
  MITK_TEST(TestNativeNrrdReloadCanBeReSavedAsDicomSegStrict);
  MITK_TEST(TestRoundTrip3D);
  MITK_TEST(TestRoundTrip4D);
  MITK_TEST(TestRoundTripWithEmptyLabel);
  MITK_TEST(TestRoundTripWithEmptyGroup);
  MITK_TEST(TestRoundTripMaxLabelValue);
  MITK_TEST(TestCustomPropertyWithoutPersistenceInfoIsDropped);
  MITK_TEST(TestReadVersion1);
  MITK_TEST(TestReadVersion2With4D);
  MITK_TEST(TestReadVersionAbove4Fails);
  MITK_TEST(TestReadGroupCountMismatchFails);
  MITK_TEST(TestReadMalformedLabelGroupsFails);
  MITK_TEST(TestReadWithoutLabelGroupsFails);
  MITK_TEST(TestReadLabelCustomPropertyOfWrongTypeFails);
  MITK_TEST(TestWriteWithoutGroupsFails);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempDir;

  static const std::string& ReaderDescription()
  {
    static const std::string description = "MITK Multilabel Segmentation";
    return description;
  }

  static const std::string& LabelGroupsKey()
  {
    static const std::string key = "org.mitk.multilabel.segmentation.labelgroups";
    return key;
  }

  std::string TempPath(const std::string& fileName) const
  {
    return (std::filesystem::path(m_TempDir) / fileName).string();
  }

  static mitk::MultiLabelSegmentation::Pointer LoadSingleSegmentation(const std::string& path)
  {
    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Loading " + path + " yields one object", std::size_t(1), loaded.size());
    auto segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Loading " + path + " yields a MultiLabelSegmentation", nullptr != segmentation);
    return segmentation;
  }

  mitk::MultiLabelSegmentation::Pointer SaveAndReload(const mitk::MultiLabelSegmentation* segmentation)
  {
    const auto path = TempPath("roundtrip.nrrd");
    mitk::IOUtil::Save(mitk::test::CloneKeepingUID(*segmentation), path);
    return LoadSingleSegmentation(path);
  }

  void AssertRoundTrip(const mitk::MultiLabelSegmentation* original)
  {
    const auto reloaded = SaveAndReload(original);
    mitk::test::AssertReloadMatchesCapabilities({ original }, { reloaded.GetPointer() },
                                                mitk::test::NativeNrrdCapabilities());
  }

  /** Saves the 3D rich fixture and returns its path, as a valid file to corrupt. */
  std::string SaveValidFile()
  {
    const auto path = TempPath("valid.nrrd");
    mitk::IOUtil::Save(mitk::test::BuildRichSegmentation({ false, false, false, false }), path);
    return path;
  }

  /** The reader itself must throw mitk::Exception mentioning text; IOUtil must report the failure too. */
  static void AssertReadFails(const std::string& path, const std::string& text)
  {
    try
    {
      mitk::test::ReadWithReader(path, ReaderDescription());
      CPPUNIT_FAIL("Reading " + path + " did not fail");
    }
    catch (const mitk::Exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE("The reader's exception should mention \"" + text + "\"; got: " + e.GetDescription(),
                             std::string(e.GetDescription()).find(text) != std::string::npos);
    }
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(path), mitk::Exception);
  }

public:
  void setUp() override
  {
    m_TempDir = mitk::IOUtil::CreateTemporaryDirectory("mitkMultiLabelSegmentationIOTest_XXXXXX");

    // A custom segmentation property reaches the NRRD header only through a persistence
    // info, and readers register infos process-wide. Registering it here keeps every test
    // independent of what an earlier test of the suite read.
    mitk::CoreServicePointer<mitk::IPropertyPersistence> persistence(mitk::CoreServices::GetPropertyPersistence());
    auto info = mitk::PropertyPersistenceInfo::New();
    info->SetNameAndKey(mitk::test::RichFixtureCustomPropertyName(), "mitk_test_custom");
    persistence->AddInfo(info, true);
  }

  void tearDown() override
  {
    mitk::CoreServicePointer<mitk::IPropertyPersistence> persistence(mitk::CoreServices::GetPropertyPersistence());
    persistence->RemoveInfo(mitk::test::RichFixtureCustomPropertyName());

    std::error_code ec;
    std::filesystem::remove_all(m_TempDir, ec);
  }

  void TestReadEmptyMultiLabelSegmentation()
  {
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_labels_meta.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading empty MultiLabel segmentation", testImages.size()==1);

    auto lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfGroups() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of labels is not correct", lsimage1->GetTotalNumberOfLabels() == 0);
  }

  void TestReadEmptyMultiLabelSegmentation_withNoMetaInformation()
  {
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_no_labels_meta.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading empty MultiLabel segmentation", testImages.size() == 1);

    auto lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfGroups() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of labels is not correct", lsimage1->GetTotalNumberOfLabels() == 0);


    testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_no_labels_meta_but_layer_dim.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading empty MultiLabel segmentation", testImages.size() == 1);

    lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfGroups() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of labels is not correct", lsimage1->GetTotalNumberOfLabels() == 0);
  }

  void TestReadEmptyMultiLabelSegmentation_withNoMetaInformation_butContent()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Loading empty segmentation with invalid pixel content did not fail.",
      mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_no_labels_meta_but_pixel_content.nrrd")),
      mitk::Exception);
  }

private:
  std::string DICOMKey(unsigned int g, unsigned int e) const
  {
    return mitk::GeneratePropertyNameForDICOMTag(g, e);
  }

  std::string ReadStringProp(const mitk::MultiLabelSegmentation *seg,
                             unsigned int g, unsigned int e) const
  {
    const auto prop = seg->GetConstProperty(DICOMKey(g, e));
    return prop.IsNotNull() ? prop->GetValueAsString() : std::string();
  }

  // Build a seg derived from a DICOM-flavoured source image: Initialize
  // copies Patient/Study/StudyID/FoR via DeriveDICOMSourceProperties;
  // the constructor mints SeriesInstanceUID. Adds one label and a
  // foreground voxel block so the native serialiser has content.
  mitk::MultiLabelSegmentation::Pointer BuildSegFromDicomSource(
    const mitk::Image *sourceImage,
    unsigned char labelValue = 1)
  {
    auto seg = mitk::MultiLabelSegmentation::New();
    seg->Initialize(sourceImage);

    auto label = mitk::Label::New();
    label->SetName("L");
    label->SetValue(labelValue);
    label->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
    label->SetAlgorithmName("NativeIOTest");
    seg->AddLabel(label, 0, true, true);

    auto groupImage = seg->GetGroupImage(0);
    mitk::ImageWriteAccessor writeAccessor(groupImage);
    auto *pixels = static_cast<mitk::Label::PixelType *>(writeAccessor.GetData());
    const auto dims = groupImage->GetDimensions();
    const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
    for (unsigned int z = 0; z < dims[2]; ++z)
    {
      pixels[z * sliceSize + 0] = labelValue;
      pixels[z * sliceSize + 1] = labelValue;
    }

    return seg;
  }

public:

  // Save a DICOM-derived seg as a native Multi-Label NRRD; reload it;
  // verify the IOD-mandated identity properties survive the round trip.
  // The native serialisation goes through MultiLabelIOHelper's JSON
  // sidecar.
  void TestDicomDerivedSegSurvivesNativeNrrdRoundTrip()
  {
    const std::string sourcePatientID = "Native-PID-001";
    const std::string sourceStudyUID = "1.2.826.0.1.3680043.10.999.20.1";
    const std::string sourceForUID = "1.2.826.0.1.3680043.10.999.20.3";
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity(
      "Native^Patient", sourcePatientID, "Native-Study",
      sourceStudyUID, "1.2.826.0.1.3680043.10.999.20.2", sourceForUID);

    auto seg = BuildSegFromDicomSource(source);
    const auto preSaveSeriesUID = ReadStringProp(seg, 0x0020, 0x000E);

    const std::string path = TempPath("native-roundtrip.nrrd");
    mitk::IOUtil::Save(seg, path);

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Native round-trip load produced exactly one BaseData",
                           loaded.size() == 1);
    auto *loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation *>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Native round-trip produced a MultiLabelSegmentation",
                           loadedSeg != nullptr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("PatientID survives native NRRD round trip",
                                 sourcePatientID, ReadStringProp(loadedSeg, 0x0010, 0x0020));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StudyInstanceUID survives native NRRD round trip",
                                 sourceStudyUID, ReadStringProp(loadedSeg, 0x0020, 0x000D));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StudyID survives native NRRD round trip",
                                 std::string("Native-Study"), ReadStringProp(loadedSeg, 0x0020, 0x0010));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("FrameOfReferenceUID survives native NRRD round trip",
                                 sourceForUID, ReadStringProp(loadedSeg, 0x0020, 0x0052));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SeriesInstanceUID (seg's own, minted at construction) survives native NRRD round trip",
                                 preSaveSeriesUID, ReadStringProp(loadedSeg, 0x0020, 0x000E));
  }

  // Load a DICOM-derived seg from native NRRD; re-save as DICOM SEG in
  // strict mode. Strict-mode Validate must pass on the reloaded seg
  // (the IOD-mandated identity tags survived the native serialisation
  // and the DeriveDICOMSourceProperties + reader assumptions hold).
  void TestNativeNrrdReloadCanBeReSavedAsDicomSegStrict()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    auto seg = BuildSegFromDicomSource(source);

    const auto nrrdPath = TempPath("native-then-seg.nrrd");
    mitk::IOUtil::Save(seg, nrrdPath);

    const auto loaded = mitk::IOUtil::Load(nrrdPath);
    auto *reloadedSeg = dynamic_cast<mitk::MultiLabelSegmentation *>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Reload produced a MultiLabelSegmentation", reloadedSeg != nullptr);

    const auto missing = mitk::DICOMSegmentationPropertyHelper::Validate(reloadedSeg);
    if (!missing.empty())
    {
      std::ostringstream diag;
      diag << "Reloaded seg must pass strict-mode Validate; missing items:";
      for (const auto &item : missing)
        diag << "\n  - " << item.description;
      CPPUNIT_FAIL(diag.str());
    }

    // The seg reloaded from native NRRD here carries its DICOM identity but no
    // SegSourceImageRelationRule connection, so re-establish the source
    // relation before the strict-mode DICOM SEG write (which needs a source to
    // reference). This test does NOT assert rule survival across serialisation
    // - that is covered by mitkSegSourceImageRelationRoundTripTest.
    mitk::SegSourceImageRelationRule::Connect(reloadedSeg, source.GetPointer());

    const auto dcmPath = TempPath("native-then-seg.dcm");
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Re-saving the native-reloaded seg as DICOM SEG in strict mode must succeed",
      mitk::IOUtil::Save(reloadedSeg, dcmPath));
  }

  void TestRoundTrip3D()
  {
    AssertRoundTrip(mitk::test::BuildRichSegmentation({ false, false, false, false }));
  }

  void TestRoundTrip4D()
  {
    AssertRoundTrip(mitk::test::BuildRichSegmentation({ true, false, false, false }));
  }

  void TestRoundTripWithEmptyLabel()
  {
    AssertRoundTrip(mitk::test::BuildRichSegmentation({ false, false, true, false }));
  }

  void TestRoundTripWithEmptyGroup()
  {
    AssertRoundTrip(mitk::test::BuildRichSegmentation({ false, false, false, true }));
  }

  void TestRoundTripMaxLabelValue()
  {
    auto segmentation = mitk::test::BuildRichSegmentation({ false, false, false, false });
    segmentation->AddLabel(mitk::Label::New(mitk::Label::MAX_LABEL_VALUE, "Maximum value"), 1, true, false);
    {
      // Index 0 lies outside every box of group 1.
      mitk::ImageWriteAccessor accessor(segmentation->GetGroupImage(1));
      static_cast<mitk::Label::PixelType*>(accessor.GetData())[0] = mitk::Label::MAX_LABEL_VALUE;
    }
    AssertRoundTrip(segmentation);
  }

  void TestCustomPropertyWithoutPersistenceInfoIsDropped()
  {
    mitk::CoreServicePointer<mitk::IPropertyPersistence> persistence(mitk::CoreServices::GetPropertyPersistence());
    persistence->RemoveInfo(mitk::test::RichFixtureCustomPropertyName());

    const auto reloaded = SaveAndReload(mitk::test::BuildRichSegmentation({ false, false, false, false }));
    CPPUNIT_ASSERT_MESSAGE("A custom property without persistence info is not written to the NRRD header",
                           reloaded->GetConstProperty(mitk::test::RichFixtureCustomPropertyName()).IsNull());
  }

  void TestReadVersion1()
  {
    const auto segmentation = LoadSingleSegmentation(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation.nrrd"));

    CPPUNIT_ASSERT_EQUAL(std::string("39335359-5483-4572-a38a-0d38a068de14"), segmentation->GetUID());
    CPPUNIT_ASSERT_EQUAL(1u, segmentation->GetNumberOfGroups());
    CPPUNIT_ASSERT(mitk::MultiLabelSegmentation::LabelValueVectorType({ 1, 2 }) == segmentation->GetLabelValuesByGroup(0));
    CPPUNIT_ASSERT(!segmentation->GetUnlabeledLabelLock());

    // Version 1 wrote colours as self-contained ColorProperty JSON.
    for (const auto& [value, name] : { std::pair<mitk::Label::PixelType, std::string>{ 1, "Label1" }, { 2, "Label2" } })
    {
      const auto label = segmentation->GetLabel(value);
      CPPUNIT_ASSERT_EQUAL(name, label->GetName());
      CPPUNIT_ASSERT_EQUAL(0.f, label->GetColor().GetRed());
      CPPUNIT_ASSERT_EQUAL(0.f, label->GetColor().GetGreen());
      CPPUNIT_ASSERT_EQUAL(0.f, label->GetColor().GetBlue());
      CPPUNIT_ASSERT_EQUAL(0.6f, label->GetOpacity());
      CPPUNIT_ASSERT(label->GetLocked());
      CPPUNIT_ASSERT(label->GetVisible());
    }

    // The file carries the IO bookkeeping of an earlier read by the legacy reader; it must not
    // surface as if this reader had produced it.
    const auto description = segmentation->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_DESCRIPTION()));
    CPPUNIT_ASSERT(description.IsNotNull());
    CPPUNIT_ASSERT_EQUAL(ReaderDescription(), description->GetValueAsString());
    const auto legacyOption = mitk::IOMetaInformationPropertyConstants::READER_OPTION_ROOT().AddElement("Multi layer handling");
    CPPUNIT_ASSERT(segmentation->GetConstProperty(mitk::PropertyKeyPathToPropertyName(legacyOption)).IsNull());
  }

  void TestReadVersion2With4D()
  {
    const auto segmentation = LoadSingleSegmentation(GetTestDataFilePath("Multilabel/MultilabelSegmentation3DplusT.nrrd"));

    CPPUNIT_ASSERT_EQUAL(std::string("a9e8b2e6-8e63-4eb3-882a-a4e248273b4f"), segmentation->GetUID());
    CPPUNIT_ASSERT_EQUAL(3u, segmentation->GetNumberOfGroups());
    CPPUNIT_ASSERT_EQUAL(std::string(""), segmentation->GetGroupName(0));
    CPPUNIT_ASSERT_EQUAL(std::string("Custom Name"), segmentation->GetGroupName(1));
    CPPUNIT_ASSERT_EQUAL(std::string(""), segmentation->GetGroupName(2));
    CPPUNIT_ASSERT(mitk::MultiLabelSegmentation::LabelValueVectorType({ 1, 2, 3, 4, 5 }) == segmentation->GetLabelValuesByGroup(0));
    CPPUNIT_ASSERT(mitk::MultiLabelSegmentation::LabelValueVectorType({ 6, 7 }) == segmentation->GetLabelValuesByGroup(1));
    CPPUNIT_ASSERT(mitk::MultiLabelSegmentation::LabelValueVectorType({ 8 }) == segmentation->GetLabelValuesByGroup(2));

    // Version 2 wrote colours as plain float arrays.
    struct Expected { mitk::Label::PixelType value; std::string name; float r, g, b; };
    const std::vector<Expected> expectedLabels = {
      { 1, "Label 1", 0.7450980544090271f, 0.f, 0.19607843458652496f },
      { 2, "Label 2", 0.9529411792755127f, 0.7647058963775635f, 0.f },
      { 3, "Label 3", 0.5529412031173706f, 0.7137255072593689f, 0.f },
      { 4, "Label 3", 0.5529412031173706f, 0.7137255072593689f, 0.f },
      { 5, "Label 4", 0.6313725709915161f, 0.7921568751335144f, 0.9450980424880981f },
      { 6, "Label 5", 0.6392157077789307f, 0.250980406999588f, 0.7254902124404907f },
      { 7, "Label 6", 0.f, 0.40392157435417175f, 0.6470588445663452f },
      { 8, "Label 7", 0.9529411792755127f, 0.5176470875740051f, 0.f },
    };
    for (const auto& expected : expectedLabels)
    {
      const auto label = segmentation->GetLabel(expected.value);
      const auto context = "label " + std::to_string(expected.value);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(context, expected.name, label->GetName());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(context, expected.r, label->GetColor().GetRed());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(context, expected.g, label->GetColor().GetGreen());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(context, expected.b, label->GetColor().GetBlue());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(context, std::to_string(expected.value), label->GetTrackingID());
    }

    const auto* timeGeometry = dynamic_cast<const mitk::ArbitraryTimeGeometry*>(segmentation->GetTimeGeometry());
    CPPUNIT_ASSERT_MESSAGE("The time geometry stays arbitrary", nullptr != timeGeometry);
    const std::vector<mitk::TimePointType> timePoints = { 0, 0.5, 1, 2, 5.5 };
    CPPUNIT_ASSERT_EQUAL(timePoints.size() - 1, static_cast<std::size_t>(timeGeometry->CountTimeSteps()));
    for (mitk::TimeStepType step = 0; step < timeGeometry->CountTimeSteps(); ++step)
    {
      CPPUNIT_ASSERT_EQUAL(timePoints[step], timeGeometry->GetMinimumTimePoint(step));
      CPPUNIT_ASSERT_EQUAL(timePoints[step + 1], timeGeometry->GetMaximumTimePoint(step));
    }
  }

  void TestReadVersionAbove4Fails()
  {
    const auto path = SaveValidFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::SetNrrdHeaderValue(lines, "org.mitk.multilabel.segmentation.version", "5");
    });
    AssertReadFails(path, "unsupported version");
  }

  void TestReadGroupCountMismatchFails()
  {
    const auto path = SaveValidFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::SetNrrdHeaderValue(lines, LabelGroupsKey(), R"([{"labels":[]},{"labels":[]},{"labels":[]}])");
    });
    AssertReadFails(path, "does not match");
  }

  void TestReadMalformedLabelGroupsFails()
  {
    const auto path = SaveValidFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::SetNrrdHeaderValue(lines, LabelGroupsKey(), R"([{"labels":)");
    });
    AssertReadFails(path, LabelGroupsKey());
  }

  void TestReadWithoutLabelGroupsFails()
  {
    const auto path = SaveValidFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::RemoveNrrdHeaderKey(lines, LabelGroupsKey());
    });
    AssertReadFails(path, LabelGroupsKey());
  }

  void TestReadLabelCustomPropertyOfWrongTypeFails()
  {
    const auto path = SaveValidFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::SetNrrdHeaderValue(lines, LabelGroupsKey(),
        R"([{"labels":[{"value":7,"name":"Liver","mitk.test.label.custom":{"type":5,"value":70}}]},{"labels":[]}])");
    });
    AssertReadFails(path, "mitk.test.label.custom");
  }

  void TestWriteWithoutGroupsFails()
  {
    auto segmentation = mitk::test::BuildRichSegmentation({ false, false, false, false });
    while (segmentation->GetNumberOfGroups() > 0)
      segmentation->RemoveGroup(0);

    try
    {
      mitk::IOUtil::Save(segmentation, TempPath("no-groups.nrrd"));
      CPPUNIT_FAIL("Saving a segmentation without groups did not fail");
    }
    catch (const mitk::Exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("The error should say that there are no groups; got: ") + e.GetDescription(),
                             std::string(e.GetDescription()).find("no groups") != std::string::npos);
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationIO)
