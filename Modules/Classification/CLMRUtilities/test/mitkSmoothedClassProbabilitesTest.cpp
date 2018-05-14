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

#include <itkSmoothedClassProbabilites.h>
#include <mitkImageCast.h>

class mitkSmoothedClassProbabilitesTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkSmoothedClassProbabilitesTestSuite  );

  MITK_TEST(TrainSVMClassifier);
  //  MITK_TEST(TestThreadedDecisionForest);

  CPPUNIT_TEST_SUITE_END();

private:

  typedef itk::Image<double,3> ImageType;
  mitk::Image::Pointer p1,p2,p3,pmask;
  ImageType::Pointer i1,i2,i3,imask;

public:


  void setUp(void)
  {

    // Load Image Data
    p1 = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Classification/Algorithm/ResultProb_Class-0_0.nrrd"));
    p2 = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Classification/Algorithm/ResultProb_Class-1_0.nrrd"));
    p3 = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Classification/Algorithm/ResultProb_Class-2_0.nrrd"));
    pmask = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Classification/Algorithm/mask.nrrd"));

    mitk::CastToItkImage(p1,i1);
    mitk::CastToItkImage(p2,i2);
    mitk::CastToItkImage(p3,i3);
    mitk::CastToItkImage(pmask,imask);
  }

  void TrainSVMClassifier()
  {
    itk::SmoothedClassProbabilites<ImageType>::Pointer filter = itk::SmoothedClassProbabilites<ImageType>::New();
    filter->SetInput(0,i1);
    filter->SetInput(1,i2);
    filter->SetInput(2,i3);
    filter->SetMaskImage(imask);
    filter->SetSigma(1.0);
    filter->Update();

    mitk::Image::Pointer out;
    mitk::CastToMitkImage(filter->GetOutput(), out);

    mitk::IOUtil::Save(out, "/Users/jc/test.nrrd");
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

MITK_TEST_SUITE_REGISTRATION(mitkSmoothedClassProbabilites)


