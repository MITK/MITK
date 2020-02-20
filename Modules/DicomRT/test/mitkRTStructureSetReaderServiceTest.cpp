/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkIOUtil.h>
#include <mitkContourModelSet.h>

class mitkRTStructureSetReaderServiceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTStructureSetReaderServiceTestSuite);
  MITK_TEST(TestStructureSets);
  CPPUNIT_TEST_SUITE_END();

private:
  std::vector<mitk::ContourModelSet::Pointer> structureSets;

public:

  void setUp() override
  {
    auto baseDataVector = mitk::IOUtil::Load(GetTestDataFilePath("RT/StructureSet/RS.dcm"));
    //16 structures are in the data set
    CPPUNIT_ASSERT_EQUAL(baseDataVector.size(), size_t(16));
    for (auto& baseData : baseDataVector) {
      auto contour = dynamic_cast<mitk::ContourModelSet*>(baseData.GetPointer());
      CPPUNIT_ASSERT_EQUAL(contour != nullptr, true);
      //only compare structs with content
      if (contour->GetSize() > 0) {
        structureSets.push_back(contour);
      }
    }
  }

  void TestStructureSets()
  {
    auto contourModelVectorCorrect = LoadGroundTruthData();

    bool equal = true;
    unsigned int comparedStructs = 0;
    for (const auto& aStructTest : structureSets)
    {
      const std::string nameTest = aStructTest->GetProperty("name")->GetValueAsString();
      for (const auto& aStructCorrect : contourModelVectorCorrect)
      {
        const std::string nameCorrect = aStructCorrect->GetProperty("name")->GetValueAsString();
        if (nameTest == nameCorrect) {
          if (!Compare(aStructTest, aStructCorrect)) {
            equal = false;
          }
          else {
            comparedStructs++;
          }
        }
      }
    }

    CPPUNIT_ASSERT(equal);
    //only 6 structure sets have content
    unsigned int structsExpected = 6;
    CPPUNIT_ASSERT_EQUAL(comparedStructs, structsExpected);

  }

  bool Compare(mitk::ContourModelSet::Pointer c1, mitk::ContourModelSet::Pointer c2) {
    if (c1->GetSize() != c2->GetSize())
    {
      MITK_INFO << "Number of ContourModelSets different" << std::endl;
      return false;
    }
    else
    {
      for (int i = 0; i < c1->GetSize(); ++i)
      {
        mitk::ContourModel::Pointer cm1 = c1->GetContourModelAt(i);
        mitk::ContourModel::Pointer cm2 = c2->GetContourModelAt(i);

        if (cm1->GetNumberOfVertices() != cm2->GetNumberOfVertices())
        {
          MITK_INFO << "Number of Vertices different" << std::endl;
          return false;
        }
        else
        {
          for (int j = 0; j < cm1->GetNumberOfVertices(); ++j)
          {
            mitk::Point3D p1 = cm1->GetVertexAt(i)->Coordinates;
            mitk::Point3D p2 = cm2->GetVertexAt(i)->Coordinates;
            if (!Equal(p1, p2, 0.001)) {
              return false;
            }
          }
        }
      }
    }
    return true;
  }

  mitk::ContourModelSet::Pointer LoadFileWithNameProperty(const std::string& filename, const std::string& propertyName) {
    auto readerOutput = mitk::IOUtil::Load(GetTestDataFilePath(filename));
    mitk::ContourModelSet::Pointer contourSet = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    contourSet->SetProperty("name", mitk::StringProperty::New(propertyName));
    return contourSet;
  }

  std::vector<mitk::ContourModelSet::Pointer> LoadGroundTruthData()
  {
    std::vector<mitk::ContourModelSet::Pointer> allStructs;

    allStructs.push_back(LoadFileWithNameProperty("RT/StructureSet/BODY.cnt_set", "BODY"));
    allStructs.push_back(LoadFileWithNameProperty("RT/StructureSet/Bladder.cnt_set", "Bladder"));
    allStructs.push_back(LoadFileWithNameProperty("RT/StructureSet/Femoral Head Lt.cnt_set", "Femoral Head Lt"));
    allStructs.push_back(LoadFileWithNameProperty("RT/StructureSet/Femoral Head RT.cnt_set", "Femoral Head RT"));
    allStructs.push_back(LoadFileWithNameProperty("RT/StructureSet/PTV.cnt_set", "PTV"));
    allStructs.push_back(LoadFileWithNameProperty("RT/StructureSet/Rectum.cnt_set", "Rectum"));

    return allStructs;
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkRTStructureSetReaderService)
