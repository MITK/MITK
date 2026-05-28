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
#include <mitkIPropertyPersistence.h>
#include <mitkImageWriteAccessor.h>
#include <mitkPropertyNameHelper.h>
#include <mitkPropertyPersistenceInfo.h>
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
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet1;
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet2;
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet2_adapted;

public:
  mitk::Label::Pointer GenerateLabel(mitk::Label::PixelType value, const std::string& name, float r, float g, float b) const
  {
    auto label = mitk::Label::New(value, name);
    mitk::Color color;
    color.SetRed(r);
    color.SetGreen(g);
    color.SetBlue(b);
    label->SetColor(color);

    return label;
  }

  void setUp() override
  {
    auto label = GenerateLabel(1, "Label 1", 0.745098054f, 0.f, 0.196078435f);
    auto label2 = GenerateLabel(2, "Label 2", 0.952941179, 0.764705896, 0);
    m_labelSet1 = { label, label2 };

    label = GenerateLabel(1, "Label 3", 0.552941203, 0.713725507, 0);
    label2 = GenerateLabel(2, "Label 4", 0.631372571, 0.792156875, 0.945098042);
    auto label3 = GenerateLabel(3, "Label 5", 0.639215708, 0.250980407, 0.725490212);
    m_labelSet2 = { label, label2, label3 };

    label = GenerateLabel(3, "Label 3", 0.552941203, 0.713725507, 0);
    label2 = GenerateLabel(4, "Label 4", 0.631372571, 0.792156875, 0.945098042);
    label3 = GenerateLabel(5, "Label 5", 0.639215708, 0.250980407, 0.725490212);
    m_labelSet2_adapted = { label, label2, label3 };
  }

  void tearDown() override
  {
    m_labelSet1.clear();
    m_labelSet2.clear();
    m_labelSet2_adapted.clear();
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
  // sidecar; this also covers Multi-Label Stack format transitively
  // since it uses the same helper.
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

    const auto tempDir = std::filesystem::temp_directory_path() / "mitkMultiLabelSegmentationIOTest";
    std::filesystem::create_directories(tempDir);
    const std::string path = (tempDir / "native-roundtrip.nrrd").string();
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

    const auto tempDir = std::filesystem::temp_directory_path() / "mitkMultiLabelSegmentationIOTest";
    std::filesystem::create_directories(tempDir);
    const auto nrrdPath = (tempDir / "native-then-seg.nrrd").string();
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

    mitk::SegSourceImageRelationRule::Connect(reloadedSeg, source.GetPointer());

    const auto dcmPath = (tempDir / "native-then-seg.dcm").string();
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Re-saving the native-reloaded seg as DICOM SEG in strict mode must succeed",
      mitk::IOUtil::Save(reloadedSeg, dcmPath));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationIO)
