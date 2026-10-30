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
#include <mitkPropertyPersistenceInfo.h>
#include <mitkIPropertyPersistence.h>
#include <mitkSegIOTestFixtures.h>

#include <filesystem>

std::string pathToImage;

class mitkLegacyLabelSetImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLegacyLabelSetImageIOTestSuite);
  MITK_TEST(TestRead3DLabelSetImage_Default);
  MITK_TEST(TestRead3DLabelSetImage_Adapt);
  MITK_TEST(TestRead3DLabelSetImage_Split);
  MITK_TEST(TestRead3DplusTLabelSetImage);
  MITK_TEST(TestReadMissingLabelDefinitionFails);
  MITK_TEST(TestReadLabelValueWithoutValueFails);
  MITK_TEST(TestReadMalformedLabelValueFails);
  MITK_TEST(TestReadLabelValueOfWrongTypeFails);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempDir;
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet1;
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet2;
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet2_adapted;

public:
  mitk::Label::Pointer GenerateLabel(mitk::Label::PixelType value, const std::string& name, float r, float g, float b, int layer) const
  {
    auto label = mitk::Label::New(value, name);
    mitk::Color color;
    color.SetRed(r);
    color.SetGreen(g);
    color.SetBlue(b);
    label->SetColor(color);
    label->SetIntProperty("layer", layer);
    return label;
  }

  void setUp() override
  {
    m_TempDir = mitk::IOUtil::CreateTemporaryDirectory("mitkLegacyLabelSetImageIOTest_XXXXXX");

    auto label = GenerateLabel(1, "Label 1", 0.745098054f, 0.f, 0.196078435f,0);
    auto label2 = GenerateLabel(2, "Label 2", 0.952941179, 0.764705896, 0, 0);
    m_labelSet1 = { label, label2 };

    label = GenerateLabel(1, "Label 3", 0.552941203, 0.713725507, 0, 1);
    label2 = GenerateLabel(2, "Label 4", 0.631372571, 0.792156875, 0.945098042, 1);
    auto label3 = GenerateLabel(3, "Label 5", 0.639215708, 0.250980407, 0.725490212, 1);
    m_labelSet2 = { label, label2, label3 };

    label = GenerateLabel(3, "Label 3", 0.552941203, 0.713725507, 0, 1);
    label2 = GenerateLabel(4, "Label 4", 0.631372571, 0.792156875, 0.945098042, 1);
    label3 = GenerateLabel(5, "Label 5", 0.639215708, 0.250980407, 0.725490212, 1);
    m_labelSet2_adapted = { label, label2, label3 };
  }

  void tearDown() override
  {
    m_labelSet1.clear();
    m_labelSet2.clear();
    m_labelSet2_adapted.clear();

    std::error_code ec;
    std::filesystem::remove_all(m_TempDir, ec);
  }

  /** A copy of a valid legacy file (one layer, labels 0 to 2 with values 0 to 2) to corrupt. */
  std::string CopyValidLegacyFile() const
  {
    const auto path = (std::filesystem::path(m_TempDir) / "legacy.nrrd").string();
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

  /** The reader itself must throw mitk::Exception mentioning text; IOUtil must report the failure too. */
  static void AssertReadFails(const std::string& path, const std::string& text)
  {
    try
    {
      mitk::test::ReadWithReader(path, "MITK LabelSetImage (legacy)");
      CPPUNIT_FAIL("Reading " + path + " did not fail");
    }
    catch (const mitk::Exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE("The reader's exception should mention \"" + text + "\"; got: " + e.GetDescription(),
                             std::string(e.GetDescription()).find(text) != std::string::npos);
    }
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(path), mitk::Exception);
  }

  
  void TestRead3DLabelSetImage_Default()
  {
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", testImages.size()==1);

    auto lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfGroups() == 2);

    auto loadedLabels = lsimage1->GetConstLabelsByValue(lsimage1->GetLabelValuesByGroup(0));
    CPPUNIT_ASSERT_MESSAGE("Error layer 0 is not equal", mitk::Equal(m_labelSet1, loadedLabels, mitk::eps, true));

    loadedLabels = lsimage1->GetConstLabelsByValue(lsimage1->GetLabelValuesByGroup(1));
    CPPUNIT_ASSERT_MESSAGE("Error layer 1 is not equal", mitk::Equal(m_labelSet2_adapted, loadedLabels, mitk::eps, true));

    CPPUNIT_ASSERT_MESSAGE("Error, read image has different UID", "c236532b-f95a-4f22-a4c6-7abe4e41ad10"== lsimage1->GetUID());
  }

  void TestRead3DLabelSetImage_Adapt()
  {
    mitk::IFileReader::Options options = { {"Multi layer handling", us::Any(std::string("Adapt label values"))} };
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"), options);

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", testImages.size() == 1);

    auto lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfGroups() == 2);

    auto loadedLabels = lsimage1->GetConstLabelsByValue(lsimage1->GetLabelValuesByGroup(0));
    CPPUNIT_ASSERT_MESSAGE("Error layer 0 is not equal", mitk::Equal(m_labelSet1, loadedLabels, mitk::eps, true));

    loadedLabels = lsimage1->GetConstLabelsByValue(lsimage1->GetLabelValuesByGroup(1));
    CPPUNIT_ASSERT_MESSAGE("Error layer 1 is not equal", mitk::Equal(m_labelSet2_adapted, loadedLabels, mitk::eps, true));

    CPPUNIT_ASSERT_MESSAGE("Error, read image has different UID", "c236532b-f95a-4f22-a4c6-7abe4e41ad10" == lsimage1->GetUID());
  }

  void TestRead3DLabelSetImage_Split()
  {
    mitk::IFileReader::Options options = { {"Multi layer handling", us::Any(std::string("Split layers"))} };
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"), options);

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", testImages.size() == 2);

    auto lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());
    auto lsimage2 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[1].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers in image 1 isnot correct", lsimage1->GetNumberOfGroups() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of layers in image 2 is not correct", lsimage2->GetNumberOfGroups() == 1);

    auto loadedLabels = lsimage1->GetConstLabelsByValue(lsimage1->GetLabelValuesByGroup(0));
    CPPUNIT_ASSERT_MESSAGE("Error layer 0 is not equal", mitk::Equal(m_labelSet1, loadedLabels, mitk::eps, true));

    loadedLabels = lsimage2->GetConstLabelsByValue(lsimage2->GetLabelValuesByGroup(0));
    CPPUNIT_ASSERT_MESSAGE("Error layer 1 is not equal", mitk::Equal(m_labelSet2, loadedLabels, mitk::eps, true));

    CPPUNIT_ASSERT_MESSAGE("Error, read image has same UID", "c236532b-f95a-4f22-a4c6-7abe4e41ad10" != lsimage1->GetUID());

  }

  void TestRead3DplusTLabelSetImage()
  {

  }

  void TestReadMissingLabelDefinitionFails()
  {
    const auto path = CopyValidLegacyFile();
    mitk::test::EditNrrdHeader(path, [](std::vector<std::string>& lines) {
      mitk::test::RemoveNrrdHeaderKey(lines, "org.mitk.label_000_00002");
    });
    AssertReadFails(path, "label_000_00002");
  }

  void TestReadLabelValueWithoutValueFails()
  {
    const auto path = CopyValidLegacyFile();
    EditLabelDefinition(path, R"(<unsigned value="1" />)", "<unsigned />");
    AssertReadFails(path, "'value'");
  }

  void TestReadMalformedLabelValueFails()
  {
    const auto path = CopyValidLegacyFile();
    EditLabelDefinition(path, R"(<unsigned value="1" />)", R"(<unsigned value="one" />)");
    AssertReadFails(path, "'value'");
  }

  void TestReadLabelValueOfWrongTypeFails()
  {
    const auto path = CopyValidLegacyFile();
    EditLabelDefinition(path, R"(type="UShortProperty"><unsigned value="1" />)", R"(type="StringProperty"><string value="1" />)");
    AssertReadFails(path, "wrong property type");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkLegacyLabelSetImageIO)
