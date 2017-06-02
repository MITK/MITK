/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include "mitkRTStructureSetReader.h"
#include <mitkIOUtil.h>

class mitkRTStructureSetReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTStructureSetReaderTestSuite);
  MITK_TEST(TestStructureSets);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {
  }

  void TestStructureSets()
  {
    auto structureSetReader = mitk::RTStructureSetReader();
    structureSetReader.SetInput(GetTestDataFilePath("RT/StructureSet/RS.dcm"));
    auto readerOutput = structureSetReader.Read();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("reader output should have one entry.", static_cast<unsigned int>(16), static_cast<unsigned int>(readerOutput.size()));

    auto contourModelVectorCorrect = LoadGroundTruthData();

    std::vector<mitk::ContourModelSet::Pointer> contourModelVectorTest;
    for (const auto& aStruct : readerOutput){
        mitk::ContourModelSet::Pointer contourModelSet = dynamic_cast<mitk::ContourModelSet*>(aStruct.GetPointer());
        //only compare structs with content
        if (contourModelSet->GetSize() > 0){
            contourModelVectorTest.push_back(contourModelSet);
        }
    }

    bool equal = true;
    for (const auto& aStructTest : contourModelVectorTest)
    {
        const std::string nameTest = aStructTest->GetProperty("name")->GetValueAsString();
        for (const auto& aStructCorrect : contourModelVectorCorrect)
        {
            const std::string nameCorrect = aStructCorrect->GetProperty("name")->GetValueAsString();
            if (nameTest == nameCorrect){
                if (!Compare(aStructTest, aStructCorrect)){
                    equal = false;
                }
            }
        }
    }

    CPPUNIT_ASSERT(equal);

  }

  bool Compare(mitk::ContourModelSet::Pointer c1, mitk::ContourModelSet::Pointer c2){
    if(c1->GetSize()!=c2->GetSize())
    {
      MITK_INFO << "Number of ContourModelSets different" << std::endl;
      return false;
    }
    else
    {
      for(int i=0;i<c1->GetSize();++i)
      {
        mitk::ContourModel::Pointer cm1 = c1->GetContourModelAt(i);
        mitk::ContourModel::Pointer cm2 = c2->GetContourModelAt(i);

        if(cm1->GetNumberOfVertices()!=cm2->GetNumberOfVertices())
        {
          MITK_INFO << "Number of Vertices different" << std::endl;
          return false;
        }
        else
        {
          for(int j=0;j<cm1->GetNumberOfVertices();++j)
          {
            mitk::Point3D p1 = cm1->GetVertexAt(i)->Coordinates;
            mitk::Point3D p2 = cm2->GetVertexAt(i)->Coordinates;
            if (!Equal(p1, p2, 0.001)){
                return false;
            }
          }
        }
      }
    }
    return true;
  }

  mitk::ContourModelSet::Pointer LoadFileWithNameProperty(const std::string& filename, const std::string& propertyName){
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

MITK_TEST_SUITE_REGISTRATION(mitkRTStructureSetReader)
