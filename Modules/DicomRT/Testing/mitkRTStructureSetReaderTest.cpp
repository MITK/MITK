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

#include "mitkTestingMacros.h"
#include <mitkTestFixture.h>

#include "mitkRTStructureSetReader.h"
#include <mitkIOUtil.h>
#include <mitkStringProperty.h>

class mitkRTStructureSetReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRTStructureSetReaderTestSuite);
//  MITK_TEST(TestBody);
  MITK_TEST(TestStructureSets);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::RTStructureSetReader::Pointer m_rtStructureReader;

public:

  void setUp()
  {
    m_rtStructureReader = mitk::RTStructureSetReader::New();
    CPPUNIT_ASSERT_MESSAGE("Failed to initialize RTStructureSetReader", m_rtStructureReader.IsNotNull());
  }

  void TestStructureSets()
  {
    std::deque<mitk::ContourModelSet::Pointer> contourModelVectorCorrect;
    std::deque<mitk::ContourModelSet::Pointer> contourModelVectorCorrectSequ;
    std::deque<mitk::DataNode::Pointer> contourModelVectorTest;
    std::deque<mitk::ContourModelSet::Pointer> contourModelVectorTestDel;

    LoadData(contourModelVectorCorrect);

     contourModelVectorTest = m_rtStructureReader->ReadStructureSet(GetTestDataFilePath("RT/StructureSet/RS.dcm").c_str());

     //Deleting all empty contourmodelsets - empty contourmodelsets cant be
     //saved so we have reference for the comparison
     for(unsigned int i=0; i<contourModelVectorTest.size();++i)
     {
       if(dynamic_cast<mitk::ContourModelSet*>(contourModelVectorTest.at(i)->GetData())->GetSize()>0){
         contourModelVectorTestDel.push_back(dynamic_cast<mitk::ContourModelSet*>(contourModelVectorTest.at(i)->GetData()));
       }
     }

     //Loop for ordering the loaded contourmodelsets(contourModelVectorCorrect)
     for(unsigned int i=0; i<contourModelVectorTestDel.size();++i)
     {
       mitk::BaseProperty::Pointer name = contourModelVectorTestDel.at(i)->GetProperty("name");
       for(unsigned int j=0; j<contourModelVectorCorrect.size();++j)
       {
         mitk::BaseProperty::Pointer tmp = contourModelVectorCorrect.at(j)->GetProperty("name");
         if(tmp->GetValueAsString().compare(name->GetValueAsString()) == 0)
           contourModelVectorCorrectSequ.push_back(contourModelVectorCorrect.at(j));
       }
     }

     //Testing wheather the two deques are equal
     bool equal = true;
     for(unsigned int i=0;i<contourModelVectorCorrectSequ.size();++i)
     {
       if(!Compare(contourModelVectorCorrectSequ.at(i),contourModelVectorTestDel.at(i)))
         equal = false;
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
          float ep = 0.001;
          for(int j=0;j<cm1->GetNumberOfVertices();++j)
          {
            mitk::Point3D p1 = cm1->GetVertexAt(i)->Coordinates;
            mitk::Point3D p2 = cm2->GetVertexAt(i)->Coordinates;
            if(abs(p1[0]-p2[0]) > ep || abs(p1[0]-p2[0]) > ep || abs(p1[0]-p2[0]) > ep)
            {
              return false;
            }
          }
        }
      }
    }
    return true;
  }

  void LoadData(std::deque<mitk::ContourModelSet::Pointer> &r)
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > readerOutput;

    readerOutput = mitk::IOUtil::Load(GetTestDataFilePath("RT/StructureSet/BODY.cnt_set"));
    mitk::ContourModelSet::Pointer cnt_set = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    cnt_set->SetProperty("name", mitk::StringProperty::New("BODY"));
    r.push_back(cnt_set);

    readerOutput = mitk::IOUtil::Load(GetTestDataFilePath("RT/StructureSet/Bladder.cnt_set"));
    cnt_set = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    cnt_set->SetProperty("name", mitk::StringProperty::New("Bladder"));
    r.push_back(cnt_set);

    readerOutput = mitk::IOUtil::Load(GetTestDataFilePath("RT/StructureSet/Femoral Head Lt.cnt_set"));
    cnt_set = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    cnt_set->SetProperty("name", mitk::StringProperty::New("Femoral Head Lt"));
    r.push_back(cnt_set);

    readerOutput = mitk::IOUtil::Load(GetTestDataFilePath("RT/StructureSet/Femoral Head RT.cnt_set"));
    cnt_set = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    cnt_set->SetProperty("name", mitk::StringProperty::New("Femoral Head RT"));
    r.push_back(cnt_set);

    readerOutput = mitk::IOUtil::Load(GetTestDataFilePath("RT/StructureSet/PTV.cnt_set"));
    cnt_set = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    cnt_set->SetProperty("name", mitk::StringProperty::New("PTV"));
    r.push_back(cnt_set);

    readerOutput = mitk::IOUtil::Load(GetTestDataFilePath("RT/StructureSet/Rectum.cnt_set"));
    cnt_set = dynamic_cast<mitk::ContourModelSet*>(readerOutput.at(0).GetPointer());
    cnt_set->SetProperty("name", mitk::StringProperty::New("Rectum"));
    r.push_back(cnt_set);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkRTStructureSetReader)
