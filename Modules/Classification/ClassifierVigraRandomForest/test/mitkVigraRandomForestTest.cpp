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

#include <vigra/random_forest_hdf5_impex.hxx>
#include <mitkImageToEigenTransform.h>
#include <mitkEigenToImageTransform.h>
#include <mitkVigraRandomForestClassifier.h>
#include <itkLabelSampler.h>
#include <itkAddImageFilter.h>
#include <mitkImageCast.h>

class mitkVigraRandomForestTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkVigraRandomForestTestSuite  );

  MITK_TEST(TrainThreadedDecisionForest);
  //  MITK_TEST(TestThreadedDecisionForest);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer inputImage;
  mitk::Image::Pointer classMask;
  mitk::Image::Pointer F1;
  mitk::Image::Pointer F2;
  mitk::Image::Pointer F3;
  Eigen::MatrixXd X;
  Eigen::MatrixXi y;
  Eigen::MatrixXd X_predict;

  mitk::VigraRandomForestClassifier::Pointer classifier;

public:

  void setUp(void)
  {



    // Load Image Data
    inputImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/InputImage.nrrd"));
    classMask = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/ClassMask.nrrd"));
    F1 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/F1.nrrd"));
    F2 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/F2.nrrd"));
    F3 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/F3.nrrd"));

    itk::Image<unsigned char,3>::Pointer itkClass;
    mitk::CastToItkImage(classMask,itkClass);

    typedef itk::Image<unsigned char,3> UCharImageType;
    auto ADDFILTER = itk::AddImageFilter<UCharImageType>::New();
    auto filter = itk::LabelSampler<UCharImageType>::New();
    filter->SetInput(itkClass);
    filter->SetAcceptRate(1.0);
    filter->SetLabel(2);
    filter->Update();
    ADDFILTER->SetInput1(filter->GetOutput());
    MITK_INFO << "Label 2: " << filter->GetNumberOfSampledVoxels();

    auto labelcountmap = filter->GetLabelVoxelCountMap();
    double rate_1 = filter->GetNumberOfSampledVoxels() / (double)labelcountmap[1];
    double rate_3 = filter->GetNumberOfSampledVoxels() / (double)labelcountmap[3];

    filter = itk::LabelSampler<itk::Image<unsigned char,3> >::New();
    filter->SetInput(itkClass);
    filter->SetAcceptRate(rate_1);
    filter->SetLabel(1);
    filter->Update();
    ADDFILTER->SetInput2(filter->GetOutput());
    ADDFILTER->Update();
    MITK_INFO << "Label 1: " << filter->GetNumberOfSampledVoxels();
    ADDFILTER->SetInput1(ADDFILTER->GetOutput());

    filter = itk::LabelSampler<itk::Image<unsigned char,3> >::New();
    filter->SetInput(itkClass);
    filter->SetAcceptRate(rate_3);
    filter->SetLabel(3);
    filter->Update();
    ADDFILTER->SetInput2(filter->GetOutput());
    ADDFILTER->Update();

    MITK_INFO << "Label 3: " << filter->GetNumberOfSampledVoxels();
//    exit(0);


    mitk::Image::Pointer sampledClassMask;
    mitk::CastToMitkImage(ADDFILTER->GetOutput(), sampledClassMask);
    Eigen::MatrixXd fea1,fea2,fea3,fea4;

    unsigned int n_features = 4; // F1,F2,F3 and inputImage
    unsigned int n_samples =  mitk::ImageToEigenTransform::countIf(sampledClassMask,[](double x){return x>0;});

    mitk::ImageToEigenTransform::transform(inputImage,sampledClassMask,fea1);
    mitk::ImageToEigenTransform::transform(F1,sampledClassMask,fea2);
    mitk::ImageToEigenTransform::transform(F2,sampledClassMask,fea3);
    mitk::ImageToEigenTransform::transform(F3,sampledClassMask,fea4);

    X = Eigen::MatrixXd(n_samples,n_features);
    X << fea1,fea2,fea3,fea4;

    mitk::ImageToEigenTransform::transform(classMask,sampledClassMask,y);

    n_features = 4;
    n_samples =  mitk::ImageToEigenTransform::countIf(classMask,[](double x){return x>0;});

    mitk::ImageToEigenTransform::transform(inputImage,classMask,fea1);
    mitk::ImageToEigenTransform::transform(F1,classMask,fea2);
    mitk::ImageToEigenTransform::transform(F2,classMask,fea3);
    mitk::ImageToEigenTransform::transform(F3,classMask,fea4);


    X_predict = Eigen::MatrixXd(n_samples,n_features);
    X_predict << fea1,fea2,fea3,fea4;

    MITK_INFO << "Shape of X [" << X.rows() << " " << X.cols() << "]";
    MITK_INFO << "Shape of X_predict [" << X_predict.rows() << " " << X_predict.cols() << "]";
    MITK_INFO << "Shape of Y [" << y.rows() << "]";

    classifier = mitk::VigraRandomForestClassifier::New();

  }

  void TrainThreadedDecisionForest()
  {

    classifier->SetTreeCount(100);
    classifier->SetMaximumTreeDepth(10);
    classifier->PrintParameter();
    classifier->Train(X,y);

    MITK_INFO << classifier->GetRandomForest().class_count();
    MITK_INFO << classifier->GetRandomForest().tree_count();

    Eigen::MatrixXi classes = classifier->Predict(X_predict);
    mitk::Image::Pointer img = mitk::EigenToImageTransform::transform(classes, classMask);
    mitk::IOUtil::Save(img,"/Users/jc/prediction.nrrd");
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


