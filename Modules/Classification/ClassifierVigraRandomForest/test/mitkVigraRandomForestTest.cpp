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
#include "mitkIOUtil.h"
//#include "mitkDecisionForest.h"


//#include "mitkDataCollection.h"
//#include "mitkCollectionReader.h"
//#include "mitkCollectionWriter.h"
//#include <itkLabelSampler.h>
//#include <mitkImageCast.h>
//#include <mitkMBIConfig.h>

#include <vigra/random_forest_hdf5_impex.hxx>

class mitkVigraRandomForestTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkVigraRandomForestTestSuite  );

//  MITK_TEST(TrainThreadedDecisionForest);
//  MITK_TEST(TestThreadedDecisionForest);

  CPPUNIT_TEST_SUITE_END();

private:
//  static std::string GetTestDataFilePath(const std::string& testData)
//  {
//    if (itksys::SystemTools::FileIsFullPath(testData.c_str())) return testData;
//    return std::string(MITK_MBI_DATA_DIR) + "/" + testData;
//  }

//  mitk::DecisionForest::Pointer m_EmptyDecisionForest, m_LoadedDecisionForest;
//  mitk::DataCollection::Pointer m_TrainDatacollection;
//  mitk::DataCollection::Pointer m_TestDatacollection;
//  std::vector<std::string> m_Selected_items;
public:

  void setUp(void)
  {
//    std::string path = GetTestDataFilePath("Classification/DataCollection/dc.xml");
//    std::vector<std::string> selected_subjects = mitk::CollectionReader::GetSubjectsList(path);

//    m_Selected_items = mitk::CollectionReader::GetItemList(path);
//    m_Selected_items.erase(std::find(m_Selected_items.begin(),m_Selected_items.end(), "ClassMask"));

    // output
//    MITK_INFO << " Load Subjects:";
//    std::for_each(selected_subjects.begin(),selected_subjects.end(), [](std::string & n){ MITK_INFO << "\t" <<n; });
//    MITK_INFO << " Load Items:";
//    std::for_each(m_Selected_items.begin(),m_Selected_items.end(), [](std::string & n){ MITK_INFO << "\t" << n; });

//    mitk::CollectionReader colReader;
//    // split to train_dc
//    m_TrainDatacollection = colReader.LoadCollection(path);
//    path = GetTestDataFilePath("Classification/DataCollection/Study/Subject/Study_Subject_ClassMask.nrrd");
//    mitk::Image::Pointer classMask = mitk::IOUtil::LoadImage(path);

//    typedef itk::Image<unsigned char, 3> UCharImageType;
//    UCharImageType::Pointer itkImage;
//    mitk::CastToItkImage(classMask,itkImage);

//    itk::LabelSampler<UCharImageType>::Pointer sampleFilter = itk::LabelSampler<UCharImageType>::New();
//    sampleFilter->SetInput(itkImage);
//    sampleFilter->SetAcceptRate(0.001);
//    sampleFilter->Update();
//    dynamic_cast<mitk::DataCollection *>(
//          dynamic_cast<mitk::DataCollection *>(
//            m_TrainDatacollection->GetData("Test-Study").GetPointer())
//          ->GetData("Test-Subject").GetPointer())
//        ->AddData(sampleFilter->GetOutput(),"SampledCalssMask");

//    m_EmptyDecisionForest = mitk::DecisionForest::New();
//    m_LoadedDecisionForest = mitk::DecisionForest::New();

//    // ToDo: use mitk::IOUtil to laod hdf decision forest
//    // ModuleActivator fail to register decision forest IO
//    path = GetTestDataFilePath("Classification/forest.hdf5");
//    vigra::RandomForest<int> rf;
//    vigra::rf_import_HDF5(rf, path);
//    m_LoadedDecisionForest->SetRandomForest(rf);
  }

  void TrainThreadedDecisionForest()
  {
//    m_EmptyDecisionForest->SetCollection(m_TrainDatacollection);
//    m_EmptyDecisionForest->SetModalities(m_Selected_items);
//    m_EmptyDecisionForest->SetMaskName("SampledCalssMask");
//    m_EmptyDecisionForest->SetTreeCount(16);
//    m_EmptyDecisionForest->TrainThreaded();
  }

  void TestThreadedDecisionForest()
  {
//    m_LoadedDecisionForest->SetCollection(m_TrainDatacollection);
//    m_LoadedDecisionForest->SetModalities(m_Selected_items);
//    m_LoadedDecisionForest->SetMaskName("ClassMask");
//    m_LoadedDecisionForest->SetResultMask("ResultMask");
//    m_LoadedDecisionForest->SetResultProb("ResultProb");
//    m_LoadedDecisionForest->TestThreaded();

    //    mitk::DataCollection::Pointer res_col = dynamic_cast<mitk::DataCollection *>(dynamic_cast<mitk::DataCollection *>(m_TrainDatacollection->GetData("Test-Study").GetPointer())->GetData("Test-Subject").GetPointer());
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultMask"),"/Users/jc/res_mask.nrrd");
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultProb_Class-0"),"/Users/jc/res_prob_0.nrrd");
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultProb_Class-1"),"/Users/jc/res_prob_1.nrrd");
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultProb_Class-2"),"/Users/jc/res_prob_2.nrrd");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkVigraRandomForest)


