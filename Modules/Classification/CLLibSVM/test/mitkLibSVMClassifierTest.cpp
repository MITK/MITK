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

//#include <vigra/random_forest_hdf5_impex.hxx>
#include <mitkLibSVMClassifier.h>
#include <itkLabelSampler.h>
#include <mitkImageCast.h>
#include <boost/algorithm/string.hpp>


class mitkLibSVMClassifierTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkLibSVMClassifierTestSuite  );

  MITK_TEST(TrainSVMClassifier);
//  MITK_TEST(TestThreadedDecisionForest);

  CPPUNIT_TEST_SUITE_END();

private:
//  typedef mitk::ImageToEigenTransform::VectorType EigenVectorType;
//  static std::string GetTestDataFilePath(const std::string& testData)
//  {
//    if (itksys::SystemTools::FileIsFullPath(testData.c_str())) return testData;
//    return std::string(MITK_MBI_DATA_DIR) + "/" + testData;
//  }

//  mitk::DecisionForest::Pointer m_EmptyDecisionForest, m_LoadedDecisionForest;
//  mitk::DataCollection::Pointer m_TrainDatacollection;
//  mitk::DataCollection::Pointer m_TestDatacollection;
//  std::vector<std::string> m_Selected_items;
  mitk::Image::Pointer inputImage;
  mitk::Image::Pointer classMask;
  mitk::Image::Pointer F1;
  mitk::Image::Pointer F2;
  mitk::Image::Pointer F3;
  Eigen::MatrixXd X;
  Eigen::MatrixXi y;
  Eigen::MatrixXd X_predict;

  mitk::LibSVMClassifier::Pointer classifier;

public:



  void MergeLabels(std::map<unsigned int, unsigned int> map, mitk::Image::Pointer img)
  {

    itk::Image<unsigned int, 3>::Pointer classImage;
    mitk::CastToItkImage(img, classImage);
    auto it = itk::ImageRegionIterator<itk::Image<unsigned int, 3> >(classImage,classImage->GetLargestPossibleRegion());
    it.GoToBegin();

    while(!it.IsAtEnd())
    {
      if(map.find(it.Value())!=map.end())
        it.Set( map[it.Value()] );
      ++it;
    }

    mitk::CastToMitkImage(classImage,img);

  }

  void setUp(void)
  {



    // Load Image Data
    inputImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/InputImage.nrrd"));
    classMask = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/ClassMask.nrrd"));
    F1 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/F1.nrrd"));

    std::map<unsigned int, unsigned int> map;
    map[0] = 0;
    map[1] = 1;
    map[2] = 1;
    map[3] = 2;
    MergeLabels(map,classMask);

    itk::Image<unsigned char,3>::Pointer itkClass;
    mitk::CastToItkImage(classMask,itkClass);

    itk::LabelSampler<itk::Image<unsigned char,3> >::Pointer filter = itk::LabelSampler<itk::Image<unsigned char,3> >::New();
    filter->SetInput(itkClass);
    filter->SetAcceptRate(0.01);
    filter->Update();

    mitk::Image::Pointer sampledClassMask;
    mitk::CastToMitkImage(filter->GetOutput(), sampledClassMask);

//    // Initialize X
//    Eigen::MatrixXd vec = mitk::ImageToEigenTransform::transform(inputImage,sampledClassMask);
//    unsigned int n_features = 4; // F1,F2,F3 and inputImage
//    unsigned int n_samples = vec.rows();

//    X = Eigen::MatrixXd(n_samples,n_features);
//    X.col(0) = vec;
//    X.col(1) = mitk::ImageToEigenTransform::transform(F1,sampledClassMask);

//    y = mitk::ImageToEigenTransform::transform(classMask,sampledClassMask);

//    vec = mitk::ImageToEigenTransform::transform(inputImage,classMask);
//    n_samples = vec.rows();

//    X_predict = Eigen::MatrixXd(n_samples,n_features);
//    X_predict.col(0) = vec;
//    X_predict.col(1) = mitk::ImageToEigenTransform::transform(F1,classMask);

//    MITK_INFO << "Shape of X [" << X.rows() << " " << X.cols() << "]";
//    MITK_INFO << "Shape of X_predict [" << X_predict.rows() << " " << X_predict.cols() << "]";
//    MITK_INFO << "Shape of Y [" << y.rows() << "]";

//    classifier = mitk::LibSVMClassifier::New();

  }

  void TrainSVMClassifier()
  {
//    classifier->Train(X,y);

//    Eigen::MatrixXd classes = classifier->Predict(X_predict);
//    mitk::Image::Pointer img = mitk::EigenToImageTransform::transform(classes, classMask);
//    mitk::IOUtil::Save(img,"/Users/jc/prediction.nrrd");
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

MITK_TEST_SUITE_REGISTRATION(mitkLibSVMClassifier)


