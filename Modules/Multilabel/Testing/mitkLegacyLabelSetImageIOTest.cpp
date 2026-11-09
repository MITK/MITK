/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkArbitraryTimeGeometry.h>
#include <mitkCoreServices.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkIOMetaInformationPropertyConstants.h>
#include <mitkItkImageIO.h>
#include <mitkPropertyPersistenceInfo.h>
#include <mitkIPropertyPersistence.h>
#include <mitkSegIOTestCapabilities.h>
#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkUIDManipulator.h>

#include <itkImageIOFactory.h>

#include <filesystem>

class mitkLegacyLabelSetImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLegacyLabelSetImageIOTestSuite);
  MITK_TEST(TestRead3DLabelSetImage_Default);
  MITK_TEST(TestRead3DLabelSetImage_Adapt);
  MITK_TEST(TestRead3DLabelSetImage_Split);
  MITK_TEST(TestRead3DplusTLabelSetImage);
  MITK_TEST(TestReadLabelKeysWithoutOrgMitkPrefix);
  MITK_TEST(TestReadFileWithoutUID);
  MITK_TEST(TestLegacyToStackCarriesNoLegacyKeys);
  MITK_TEST(TestLegacyToNativeCarriesNoLegacyKeys);
  MITK_TEST(TestReadUnknownMultiLayerHandlingFails);
  MITK_TEST(TestReadLayerCountMismatchFails);
  MITK_TEST(TestReadMissingLabelDefinitionFails);
  MITK_TEST(TestReadLabelValueWithoutValueFails);
  MITK_TEST(TestReadMalformedLabelValueFails);
  MITK_TEST(TestReadLabelValueOfWrongTypeFails);
  MITK_TEST(TestReadOutdatedLabelPropertiesAreDiscarded);
  MITK_TEST(TestReadStampedLegacyFileWarnsOnce);
  MITK_TEST(TestReadStampOnUnlabeledDefinitionDoesNotWarn);
  CPPUNIT_TEST_SUITE_END();

private:
  /** A label as the header of a legacy file defines it. */
  struct LegacyLabel
  {
    mitk::Label::PixelType value;
    std::string name;
    float red;
    float green;
    float blue;
  };

  /** A layer of a legacy file: the component of the raw image that holds its voxels, and its labels. */
  struct LegacyLayer
  {
    unsigned int component;
    std::vector<LegacyLabel> labels;
  };

  using DICOMTags = std::vector<std::pair<std::string, std::string>>;

  std::string m_TempDir;
  bool m_AddedModalityPersistence = false;

  static mitk::IFileReader::Options MultiLayerHandling(const std::string& strategy)
  {
    return { { "Multi layer handling", us::Any(strategy) } };
  }

  /** LegacyLabelSetTestImage3D.nrrd and LegacyLabelSetTestImage3DplusT.nrrd define the same layers. */
  static std::vector<LegacyLayer> LegacyTestImageLayers()
  {
    return {
      { 0, { { 1, "Label 1", 0.745098054f, 0.f, 0.196078435f },
             { 2, "Label 2", 0.952941179f, 0.764705896f, 0.f } } },
      { 1, { { 1, "Label 3", 0.552941203f, 0.713725507f, 0.f },
             { 2, "Label 4", 0.631372571f, 0.792156875f, 0.945098042f },
             { 3, "Label 5", 0.639215708f, 0.250980407f, 0.725490212f } } } };
  }

  static DICOMTags LegacyTestImageDICOMTags()
  {
    return { { "DICOM.0008.0060", "SEG" }, { "DICOM.0008.103E", "MITK Segmentation" },
             { "DICOM.0010.0010", "NO NAME" }, { "DICOM.0010.0020", "NO NAME" }, { "DICOM.0010.0040", "U" } };
  }

  /** Identity directions and zero origin, as every legacy fixture has. */
  static mitk::TimeGeometry::Pointer BuildTimeGeometry(const std::array<unsigned int, 3>& size,
                                                       const std::array<mitk::ScalarType, 3>& spacing,
                                                       const std::vector<mitk::TimePointType>& timePoints = {})
  {
    auto image = mitk::Image::New();
    unsigned int dimensions[3] = { size[0], size[1], size[2] };
    image->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dimensions);
    mitk::Vector3D imageSpacing;
    mitk::FillVector3D(imageSpacing, spacing[0], spacing[1], spacing[2]);
    image->GetGeometry()->SetSpacing(imageSpacing);

    if (timePoints.empty())
      return image->GetTimeGeometry()->Clone();

    auto timeGeometry = mitk::ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();
    for (std::size_t step = 0; step + 1 < timePoints.size(); ++step)
      timeGeometry->AppendNewTimeStepClone(image->GetGeometry(), timePoints[step], timePoints[step + 1]);
    timeGeometry->UpdateBoundingBox();
    return timeGeometry.GetPointer();
  }

  /** The raw voxels of a legacy file, read by ITK without the legacy reader; one component per layer. */
  static mitk::Image::Pointer ReadRawImage(const std::string& path)
  {
    auto imageIO = itk::ImageIOFactory::CreateImageIO(path.c_str(), itk::CommonEnums::IOFileMode::ReadMode);
    CPPUNIT_ASSERT_MESSAGE("ITK cannot read " + path, imageIO.IsNotNull());
    return mitk::ItkImageIO::LoadRawMitkImageFromImageIO(imageIO, path);
  }

  /**
   * The segmentation a legacy file must read as: geometry, labels and properties as the
   * caller transcribed them from the header, one group per given layer, and the voxels of
   * each layer from the raw image, raised by the layer's value offset. Raw values that are
   * no label of their layer become unlabeled, or stay as they are with keepValuesOutsideLabels.
   */
  static mitk::MultiLabelSegmentation::Pointer BuildExpected(const std::string& path,
                                                             const mitk::TimeGeometry* geometry,
                                                             const std::vector<LegacyLayer>& layers,
                                                             const std::vector<mitk::Label::PixelType>& valueOffsets,
                                                             const DICOMTags& dicomTags,
                                                             const std::string& uid,
                                                             bool keepValuesOutsideLabels = false)
  {
    const auto raw = ReadRawImage(path);
    const auto components = raw->GetPixelType().GetNumberOfComponents();
    std::size_t voxelCount = 1;
    for (unsigned int axis = 0; axis < raw->GetDimension(); ++axis)
      voxelCount *= raw->GetDimension(axis);
    mitk::ImageReadAccessor rawAccessor(raw);
    const auto* rawVoxels = static_cast<const mitk::Label::PixelType*>(rawAccessor.GetData());

    auto expected = mitk::MultiLabelSegmentation::New();
    expected->Initialize(geometry, true, false);

    for (std::size_t layerIndex = 0; layerIndex < layers.size(); ++layerIndex)
    {
      const auto& layer = layers[layerIndex];
      const auto offset = valueOffsets[layerIndex];
      const auto group = expected->AddGroup();

      for (const auto& legacyLabel : layer.labels)
      {
        auto label = mitk::Label::New(static_cast<mitk::Label::PixelType>(legacyLabel.value + offset), legacyLabel.name);
        mitk::Color color;
        color.Set(legacyLabel.red, legacyLabel.green, legacyLabel.blue);
        label->SetColor(color);
        // Every label of the legacy fixtures is locked, visible and has an opacity of 0.6.
        label->SetOpacity(0.6f);
        label->SetLocked(true);
        label->SetVisible(true);
        expected->AddLabel(label, group, false, false);
      }

      mitk::ImageWriteAccessor groupAccessor(expected->GetGroupImage(group));
      auto* groupVoxels = static_cast<mitk::Label::PixelType*>(groupAccessor.GetData());
      for (std::size_t i = 0; i < voxelCount; ++i)
      {
        const auto value = rawVoxels[i * components + layer.component];
        const auto isLabel = std::any_of(layer.labels.begin(), layer.labels.end(),
          [value](const LegacyLabel& label) { return label.value == value; });
        if (isLabel)
          groupVoxels[i] = static_cast<mitk::Label::PixelType>(value + offset);
        else
          groupVoxels[i] = keepValuesOutsideLabels ? value : mitk::MultiLabelSegmentation::UNLABELED_VALUE;
      }
    }

    for (const auto& [key, value] : dicomTags)
      expected->SetProperty(key, mitk::TemporoSpatialStringProperty::New(value));

    if (!uid.empty())
    {
      mitk::UIDManipulator uidManipulator(expected);
      uidManipulator.SetUID(uid);
    }

    return expected;
  }

  static void AssertMatches(const mitk::MultiLabelSegmentation* expected,
                            const mitk::BaseData* loaded,
                            bool compareUID,
                            const std::string& context)
  {
    const auto* actual = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded);
    CPPUNIT_ASSERT_MESSAGE(context + " yields a MultiLabelSegmentation", nullptr != actual);

    mitk::test::SegComparisonOptions options{};
    // No legacy file carries a series instance UID, so every segmentation mints its own.
    options.notComparedKeys.insert("DICOM.0020.000E");
    if (!compareUID)
      options.notCompared.insert(mitk::test::NotCompared::SegmentationUID);

    const auto differences = mitk::test::CompareSegmentations(*expected, *actual, options);
    CPPUNIT_ASSERT_MESSAGE(context + " differs from its header:" + mitk::test::JoinDifferences(differences),
                           differences.empty());
  }

  static const std::string& ReaderDescription()
  {
    static const std::string description = "MITK LabelSetImage (legacy)";
    return description;
  }

  std::string TempPath(const std::string& fileName) const
  {
    return (std::filesystem::path(m_TempDir) / fileName).string();
  }

  /** A copy of a valid legacy file (one layer, labels 0 to 2 with values 0 to 2) to corrupt. */
  std::string CopyValidLegacyFile() const
  {
    const auto path = this->TempPath("legacy.nrrd");
    std::filesystem::copy_file(GetTestDataFilePath("Multilabel/StampResultBasedOnEmptyML.nrrd"), path);
    return path;
  }

  /** Replaces text in the XML definition of label 1 of layer 0. */
  static void EditLabelDefinition(const std::string& path, const std::string& oldText, const std::string& newText)
  {
    mitk::test::EditNrrdHeader(path, [&oldText, &newText](std::vector<std::string>& lines) {
      const std::string prefix = "org.mitk.label_000_00001:=";
      for (auto& line : lines)
      {
        if (line.compare(0, prefix.size(), prefix) == 0)
        {
          const auto position = line.find(oldText);
          CPPUNIT_ASSERT_MESSAGE("Label definition lacks " + oldText, position != std::string::npos);
          line.replace(position, oldText.size(), newText);
          return;
        }
      }
      CPPUNIT_FAIL("Fixture lacks the definition of label 1");
    });
  }

public:
  void setUp() override
  {
    m_TempDir = mitk::IOUtil::CreateTemporaryDirectory("mitkLegacyLabelSetImageIOTest_XXXXXX");
  }

  void tearDown() override
  {
    mitk::CoreServicePointer<mitk::IPropertyPersistence> persistence(mitk::CoreServices::GetPropertyPersistence());
    persistence->RemoveInfo(mitk::test::RichFixtureCustomPropertyName());
    if (m_AddedModalityPersistence)
    {
      persistence->RemoveInfo("modality");
      m_AddedModalityPersistence = false;
    }

    std::error_code ec;
    std::filesystem::remove_all(m_TempDir, ec);
  }

  void TestRead3DLabelSetImage_Default()
  {
    const auto path = GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd");
    const auto expected = BuildExpected(path, BuildTimeGeometry({ 30, 20, 10 }, { 1, 1, 1 }), LegacyTestImageLayers(),
                                        { 0, 2 }, LegacyTestImageDICOMTags(), "c236532b-f95a-4f22-a4c6-7abe4e41ad10");

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    AssertMatches(expected, loaded.front(), true, "LegacyLabelSetTestImage3D.nrrd without options");
  }

  void TestRead3DLabelSetImage_Adapt()
  {
    // Adapt raises the values of each layer by the maximum value of the layers before it.
    const auto path = GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd");
    const auto expected = BuildExpected(path, BuildTimeGeometry({ 30, 20, 10 }, { 1, 1, 1 }), LegacyTestImageLayers(),
                                        { 0, 2 }, LegacyTestImageDICOMTags(), "c236532b-f95a-4f22-a4c6-7abe4e41ad10");

    const auto loaded = mitk::IOUtil::Load(path, MultiLayerHandling("Adapt label values"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    AssertMatches(expected, loaded.front(), true, "LegacyLabelSetTestImage3D.nrrd, adapted");
  }

  void TestRead3DLabelSetImage_Split()
  {
    const auto path = GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd");
    const auto geometry = BuildTimeGeometry({ 30, 20, 10 }, { 1, 1, 1 });
    const auto layers = LegacyTestImageLayers();

    const auto loaded = mitk::IOUtil::Load(path, MultiLayerHandling("Split layers"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), loaded.size());
    for (std::size_t layer = 0; layer < layers.size(); ++layer)
    {
      const auto expected = BuildExpected(path, geometry, { layers[layer] }, { 0 }, LegacyTestImageDICOMTags(), "");
      AssertMatches(expected, loaded[layer], false, "LegacyLabelSetTestImage3D.nrrd, split, layer " + std::to_string(layer));
    }

    // Each split segmentation is a new one; none may claim the identity of the file.
    CPPUNIT_ASSERT(loaded[0]->GetUID() != "c236532b-f95a-4f22-a4c6-7abe4e41ad10");
    CPPUNIT_ASSERT(loaded[1]->GetUID() != "c236532b-f95a-4f22-a4c6-7abe4e41ad10");
    CPPUNIT_ASSERT(loaded[0]->GetUID() != loaded[1]->GetUID());
  }

  void TestRead3DplusTLabelSetImage()
  {
    const auto path = GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3DplusT.nrrd");
    const auto expected = BuildExpected(path, BuildTimeGeometry({ 30, 20, 10 }, { 1, 1, 1 }, { 0, 0.5, 1, 2, 5.5 }),
                                        LegacyTestImageLayers(), { 0, 2 }, LegacyTestImageDICOMTags(),
                                        "9c2a4e7a-1bf6-460a-bc31-5249b87a5bf2");

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    AssertMatches(expected, loaded.front(), true, "LegacyLabelSetTestImage3DplusT.nrrd");
  }

  void TestReadLabelKeysWithoutOrgMitkPrefix()
  {
    // Older files name the label definitions "label_*" instead of "org.mitk.label_*" and
    // write integer properties as <int>.
    const auto path = GetTestDataFilePath("Multilabel/Pic3DAsMultilabelImage.nrrd");
    const auto expected = BuildExpected(path, BuildTimeGeometry({ 256, 256, 49 }, { 1, 1, 3 }),
      { { 0, { { 1, "Liver", 1.f, 0.f, 0.f },
               { 2, "Fat", 0.929411768913269f, 1.f, 0.160784319043159f },
               { 3, "Lung", 0.419607847929001f, 0.862745106220245f, 1.f } } } },
      { 0 }, {}, "");

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    AssertMatches(expected, loaded.front(), false, "Pic3DAsMultilabelImage.nrrd");
  }

  void TestReadFileWithoutUID()
  {
    // The file also defines a label for unlabeled pixels ("Exterior", value 0); the reader drops it.
    // Four voxels hold values no label defines (54, 344, 733, 19520). Such values are invalid and
    // should be cleared on read; the reader keeps them, a known defect, which the last argument of
    // BuildExpected (keepValuesOutsideLabels) pins. Once the reader clears them, this test fails:
    // drop the argument, and the parameter with it, since this test is its only user.
    const auto path = GetTestDataFilePath("Multilabel/StampResultBasedOnEmptyML.nrrd");
    const auto expected = BuildExpected(path, BuildTimeGeometry({ 50, 50, 50 }, { 1, 1, 1 }),
      { { 0, { { 1, "Label1", 0.f, 0.f, 0.f }, { 2, "Label2", 0.f, 0.f, 0.f } } } }, { 0 }, {}, "", true);

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    AssertMatches(expected, loaded.front(), false, "StampResultBasedOnEmptyML.nrrd");
  }

  void TestLegacyToStackCarriesNoLegacyKeys()
  {
    auto segmentation = mitk::test::LoadSingleSegmentation(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"));
    // Set, so that the stack's documented loss of the lock has something to change.
    segmentation->SetUnlabeledLabelLock(true);

    const auto path = this->TempPath("legacy.mitklabel.json");
    mitk::IOUtil::Save(segmentation, path, { { "Save strategy", us::Any(std::string("group")) } });

    const auto content = mitk::test::ReadJsonFile(path);
    for (const auto& [type, properties] : content["properties"].items())
    {
      for (const auto& [key, value] : properties.items())
      {
        CPPUNIT_ASSERT_MESSAGE("Only the file's DICOM tags may reach the stack; found " + key + " (" + type + ")",
                               key.compare(0, 6, "DICOM.") == 0);
      }
    }
    for (const auto& group : content["groups"])
    {
      for (const auto& label : group["labels"])
        CPPUNIT_ASSERT_MESSAGE("The legacy layer index reached the stack: " + label.dump(), !label.contains("layer"));
    }

    const auto reloaded = mitk::test::LoadSingleSegmentation(path);
    mitk::test::AssertReloadMatchesCapabilities({ segmentation.GetPointer() }, { reloaded.GetPointer() },
                                                mitk::test::StackCapabilities());
  }

  void TestLegacyToNativeCarriesNoLegacyKeys()
  {
    // A plain NRRD read earlier in the process registers a persistence info for "modality";
    // with it, a "modality" property leaking from the legacy header would overwrite the
    // native writer's own and make the native file a legacy one.
    mitk::CoreServicePointer<mitk::IPropertyPersistence> persistence(mitk::CoreServices::GetPropertyPersistence());
    if (persistence->GetInfo("modality", false).empty())
    {
      auto info = mitk::PropertyPersistenceInfo::New();
      info->SetNameAndKey("modality", "modality");
      persistence->AddInfo(info);
      m_AddedModalityPersistence = true;
    }

    auto segmentation = mitk::test::LoadSingleSegmentation(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"));
    // A typed custom property, so that the native format's documented loss has something to change.
    auto customInfo = mitk::PropertyPersistenceInfo::New();
    customInfo->SetNameAndKey(mitk::test::RichFixtureCustomPropertyName(), "mitk_test_custom");
    persistence->AddInfo(customInfo, true);
    segmentation->SetProperty(mitk::test::RichFixtureCustomPropertyName(), mitk::IntProperty::New(42));

    const auto path = this->TempPath("legacy.nrrd");
    mitk::IOUtil::Save(segmentation, path);
    const auto reloaded = mitk::test::LoadSingleSegmentation(path);

    const auto readerDescription = reloaded->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_DESCRIPTION()));
    CPPUNIT_ASSERT(readerDescription.IsNotNull());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The saved file is read as a native one", std::string("MITK Multilabel Segmentation"),
                                 readerDescription->GetValueAsString());
    mitk::test::AssertReloadMatchesCapabilities({ segmentation.GetPointer() }, { reloaded.GetPointer() },
                                                mitk::test::NativeNrrdCapabilities());
  }

  void TestReadUnknownMultiLayerHandlingFails()
  {
    try
    {
      mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"),
                         MultiLayerHandling("Merge layers"));
      CPPUNIT_FAIL("Reading with an unknown multi layer handling did not fail");
    }
    catch (const mitk::Exception& e)
    {
      const std::string message = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("The error should name the option and the value; got: " + message,
                             message.find("Multi layer handling") != std::string::npos &&
                             message.find("Merge layers") != std::string::npos);
    }
  }

  void TestReadLayerCountMismatchFails()
  {
    const auto path = this->CopyValidLegacyFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::SetNrrdHeaderValue(lines, "layers", "2");
    });
    mitk::test::AssertReadFails(path, ReaderDescription(), "does not match");
  }

  void TestReadMissingLabelDefinitionFails()
  {
    const auto path = this->CopyValidLegacyFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::RemoveNrrdHeaderKey(lines, "org.mitk.label_000_00002");
    });
    mitk::test::AssertReadFails(path, ReaderDescription(), "label_000_00002");
  }

  void TestReadLabelValueWithoutValueFails()
  {
    const auto path = this->CopyValidLegacyFile();
    EditLabelDefinition(path, R"(<unsigned value="1" />)", "<unsigned />");
    mitk::test::AssertReadFails(path, ReaderDescription(), "'value'");
  }

  void TestReadMalformedLabelValueFails()
  {
    const auto path = this->CopyValidLegacyFile();
    EditLabelDefinition(path, R"(<unsigned value="1" />)", R"(<unsigned value="one" />)");
    mitk::test::AssertReadFails(path, ReaderDescription(), "'value'");
  }

  void TestReadLabelValueOfWrongTypeFails()
  {
    const auto path = this->CopyValidLegacyFile();
    EditLabelDefinition(path, R"(type="UShortProperty"><unsigned value="1" />)", R"(type="StringProperty"><string value="1" />)");
    mitk::test::AssertReadFails(path, ReaderDescription(), "wrong property type");
  }

  void TestReadOutdatedLabelPropertiesAreDiscarded()
  {
    // The algorithm type as MITK stamped it on every label before 2025.12.
    const auto path = this->CopyValidLegacyFile();
    EditLabelDefinition(path, "</Label>",
      R"(<property key="DICOM.0062.0002.0062.0008" type="TemporoSpatialStringProperty">)"
      R"(<temporo_spatial_string>{"values":[{"z":0, "t":0, "value":"SEMIAUTOMATIC"}]}</temporo_spatial_string>)"
      R"(</property></Label>)");

    mitk::test::WarningRecorder warnings("DICOM.0062.0002.");
    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT(nullptr != segmentation);

    CPPUNIT_ASSERT_MESSAGE("Outdated label properties survived the read",
                           mitk::test::FindOutdatedLabelProperties(*segmentation).empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One warning per file", std::size_t(1), warnings.GetWarnings().size());
    CPPUNIT_ASSERT_MESSAGE("The warning names the file: " + warnings.GetWarnings().front(),
                           warnings.GetWarnings().front().find("legacy.nrrd") != std::string::npos);
  }

  void TestReadStampedLegacyFileWarnsOnce()
  {
    // Every label of both layers carries the stamps; the values repeat across the layers.
    mitk::test::WarningRecorder warnings("DICOM.0062.0002.");
    const auto loaded = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3DplusT.nrrd"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT(nullptr != segmentation);

    CPPUNIT_ASSERT_MESSAGE("Outdated label properties survived the read",
                           mitk::test::FindOutdatedLabelProperties(*segmentation).empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One warning per file", std::size_t(1), warnings.GetWarnings().size());
    CPPUNIT_ASSERT_MESSAGE("The warning counts the labels: " + warnings.GetWarnings().front(),
                           warnings.GetWarnings().front().find("labels affected: 5") != std::string::npos);
  }

  void TestReadStampOnUnlabeledDefinitionDoesNotWarn()
  {
    // Only the definition for unlabeled pixels carries the stamps, and the reader drops that
    // definition, so no saved file could carry them on.
    mitk::test::WarningRecorder warnings("DICOM.0062.0002.");
    mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"));
    CPPUNIT_ASSERT(warnings.GetWarnings().empty());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkLegacyLabelSetImageIO)
