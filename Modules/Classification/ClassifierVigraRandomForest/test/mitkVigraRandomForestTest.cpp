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
  typedef mitk::ImageToEigenTransform::MatrixType MatrixType;

  mitk::Image::Pointer inputImage;
  mitk::Image::Pointer classMask;
  mitk::Image::Pointer F1;
  mitk::Image::Pointer F2;
  mitk::Image::Pointer F3;
  MatrixType X;
  MatrixType y;
  MatrixType X_predict;

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


//    for(const auto & pair : filter->GetLabelVoxelCountMap())
//      MITK_INFO << pair.first << " " << pair.second;

    mitk::Image::Pointer sampledClassMask;
    mitk::CastToMitkImage(ADDFILTER->GetOutput(), sampledClassMask);

    // Initialize X
    MatrixType vec = mitk::ImageToEigenTransform::transform(inputImage,sampledClassMask);
    unsigned int n_features = 4; // F1,F2,F3 and inputImage
    unsigned int n_samples = vec.rows();

    X = MatrixType(n_samples,n_features);
    X.col(0) = vec;
    X.col(1) = mitk::ImageToEigenTransform::transform(F1,sampledClassMask);
    X.col(2) = mitk::ImageToEigenTransform::transform(F2,sampledClassMask);
    X.col(3) = mitk::ImageToEigenTransform::transform(F3,sampledClassMask);

//    MatrixType X2(n_samples*2,n_features);
//    X2 << X , X;

    y = mitk::ImageToEigenTransform::transform(classMask,sampledClassMask);

    vec = mitk::ImageToEigenTransform::transform(inputImage,classMask);
    n_samples = vec.rows();

    X_predict = MatrixType(n_samples,n_features);
    X_predict.col(0) = vec;
    X_predict.col(1) = mitk::ImageToEigenTransform::transform(F1,classMask);
    X_predict.col(2) = mitk::ImageToEigenTransform::transform(F2,classMask);
    X_predict.col(3) = mitk::ImageToEigenTransform::transform(F3,classMask);

    MITK_INFO << "Shape of X [" << X.rows() << " " << X.cols() << "]";
    MITK_INFO << "Shape of X_predict [" << X_predict.rows() << " " << X_predict.cols() << "]";
    MITK_INFO << "Shape of Y [" << y.rows() << "]";

    classifier = mitk::VigraRandomForestClassifier::New();

  }

  void TrainThreadedDecisionForest()
  {

    classifier->SetTreeCount(100);
    classifier->SetTreeDepth(10);
    classifier->PrintParameter();
    classifier->Train(X,y);

    MITK_INFO << classifier->GetRandomForest().class_count();
    MITK_INFO << classifier->GetRandomForest().tree_count();

    MatrixType classes = classifier->Predict(X_predict);
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


