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
#include <mitkFiberBundle.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>
#include <omp.h>
#include <itkFitFibersToImageFilter.h>
#include <mitkTestFixture.h>
#include <mitkPeakImage.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <itkImageFileWriter.h>

class mitkFiberFitTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkFiberFitTestSuite);
  MITK_TEST(Fit1);
  MITK_TEST(Fit2);
  MITK_TEST(Fit3);
  MITK_TEST(Fit4);
  MITK_TEST(Fit5);
  MITK_TEST(Fit6);
  CPPUNIT_TEST_SUITE_END();

  typedef itk::Image<float, 3> ItkFloatImgType;

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/

  typedef itk::FitFibersToImageFilter FitterType;
  FitterType::Pointer fitter;

public:

  mitk::FiberBundle::Pointer LoadFib(std::string fib_name)
  {
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberFit/" + fib_name));
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
    return fib;
  }

  void setUp() override
  {
    std::vector<mitk::FiberBundle::Pointer> tracts;
    tracts.push_back(LoadFib("Cluster_0.fib"));
    tracts.push_back(LoadFib("Cluster_1.fib"));
    tracts.push_back(LoadFib("Cluster_2.fib"));
    tracts.push_back(LoadFib("Cluster_3.fib"));
    tracts.push_back(LoadFib("Cluster_4.fib"));

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image"}, {});
    mitk::PeakImage::Pointer peaks = mitk::IOUtil::Load<mitk::PeakImage>(GetTestDataFilePath("DiffusionImaging/FiberFit/csd_peak_image.nii.gz"), &functor);


    typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(peaks);
    caster->Update();
    mitk::PeakImage::ItkPeakImageType::Pointer peak_image = caster->GetOutput();

    fitter = FitterType::New();
    fitter->SetPeakImage(peak_image);
    fitter->SetTractograms(tracts);
  }

  void tearDown() override
  {

  }

  void CompareFibs(mitk::FiberBundle::Pointer test, mitk::FiberBundle::Pointer ref, std::string out_name)
  {
    vtkSmartPointer<vtkFloatArray> weights = test->GetFiberWeights();
    vtkSmartPointer<vtkFloatArray> ref_weights = ref->GetFiberWeights();

    CPPUNIT_ASSERT_MESSAGE("Number of weights should be equal", weights->GetSize()==ref_weights->GetSize());

    for (int i=0; i<weights->GetSize(); ++i)
    {
      if (ref_weights->GetValue(i)>0)
      {
        if (fabs( weights->GetValue(i)/ref_weights->GetValue(i)-1 )>0.01)
        {
          mitk::IOUtil::Save(test, mitk::IOUtil::GetTempPath()+out_name);
          CPPUNIT_ASSERT_MESSAGE("Weights should be equal", false);
        }
      }
      else if (weights->GetValue(i)>0)
      {
        mitk::IOUtil::Save(test, mitk::IOUtil::GetTempPath()+out_name);
        CPPUNIT_ASSERT_MESSAGE("Weights should be equal", false);
      }
    }
  }

  void CompareImages(mitk::PeakImage::ItkPeakImageType::Pointer testImage, std::string name)
  {
    typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/FiberFit/out/" + name)));
    caster->Update();
    mitk::PeakImage::ItkPeakImageType::Pointer refImage = caster->GetOutput();


    itk::ImageRegionConstIterator< mitk::PeakImage::ItkPeakImageType > it1(testImage, testImage->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator< mitk::PeakImage::ItkPeakImageType > it2(refImage, refImage->GetLargestPossibleRegion());

    while(!it1.IsAtEnd())
    {
      if (it2.Get()>0.0001)
      {
        if (fabs( it1.Get()/it2.Get()-1 )>0.01)
        {
          itk::ImageFileWriter< mitk::PeakImage::ItkPeakImageType >::Pointer writer = itk::ImageFileWriter< mitk::PeakImage::ItkPeakImageType >::New();
          writer->SetInput(testImage);
          writer->SetFileName(mitk::IOUtil::GetTempPath()+name);
          writer->Update();

          MITK_INFO << it1.Get() << " - " << it2.Get();

          CPPUNIT_ASSERT_MESSAGE("Peak images should be equal 1", false);
        }
      }
      else if (it1.Get()>0.0001)
      {
        itk::ImageFileWriter< mitk::PeakImage::ItkPeakImageType >::Pointer writer = itk::ImageFileWriter< mitk::PeakImage::ItkPeakImageType >::New();
        writer->SetInput(testImage);
        writer->SetFileName(mitk::IOUtil::GetTempPath()+name);
        writer->Update();

        CPPUNIT_ASSERT_MESSAGE("Peak images should be equal 2", false);
      }

      ++it1;
      ++it2;
    }
  }

  void Fit1()
  {
    omp_set_num_threads(1);
    fitter->SetLambda(0.1);
    fitter->SetFilterOutliers(false);
    fitter->SetRegularization(VnlCostFunction::NONE);
    fitter->Update();

    std::vector< mitk::FiberBundle::Pointer > output_tracts = fitter->GetTractograms();
    mitk::FiberBundle::Pointer test = mitk::FiberBundle::New();
    test = test->AddBundles(output_tracts);
    mitk::FiberBundle::Pointer ref = LoadFib("out/NONE_fitted.fib");
    CompareFibs(test, ref, "NONE_fitted.fib");
    CompareImages(fitter->GetFittedImage(), "NONE_fitted_image.nrrd");
    CompareImages(fitter->GetResidualImage(), "NONE_residual_image.nrrd");
  }

  void Fit2()
  {
    omp_set_num_threads(1);
    fitter->SetLambda(0.1);
    fitter->SetFilterOutliers(false);
    fitter->SetRegularization(VnlCostFunction::MSM);
    fitter->Update();

    std::vector< mitk::FiberBundle::Pointer > output_tracts = fitter->GetTractograms();
    mitk::FiberBundle::Pointer test = mitk::FiberBundle::New();
    test = test->AddBundles(output_tracts);
    mitk::FiberBundle::Pointer ref = LoadFib("out/MSM_fitted.fib");
    CompareFibs(test, ref, "MSM_fitted.fib");
    CompareImages(fitter->GetFittedImage(), "MSM_fitted_image.nrrd");
    CompareImages(fitter->GetResidualImage(), "MSM_residual_image.nrrd");
  }

  void Fit3()
  {
    omp_set_num_threads(1);
    fitter->SetLambda(0.1);
    fitter->SetFilterOutliers(false);
    fitter->SetRegularization(VnlCostFunction::VARIANCE);
    fitter->Update();

    std::vector< mitk::FiberBundle::Pointer > output_tracts = fitter->GetTractograms();
    mitk::FiberBundle::Pointer test = mitk::FiberBundle::New();
    test = test->AddBundles(output_tracts);
    mitk::FiberBundle::Pointer ref = LoadFib("out/MSE_fitted.fib");
    CompareFibs(test, ref, "MSE_fitted.fib");
    CompareImages(fitter->GetFittedImage(), "MSE_fitted_image.nrrd");
    CompareImages(fitter->GetResidualImage(), "MSE_residual_image.nrrd");
  }

  void Fit4()
  {
    omp_set_num_threads(1);
    fitter->SetLambda(0.1);
    fitter->SetFilterOutliers(false);
    fitter->SetRegularization(VnlCostFunction::VOXEL_VARIANCE);
    fitter->Update();

    std::vector< mitk::FiberBundle::Pointer > output_tracts = fitter->GetTractograms();
    mitk::FiberBundle::Pointer test = mitk::FiberBundle::New();
    test = test->AddBundles(output_tracts);
    mitk::FiberBundle::Pointer ref = LoadFib("out/LocalMSE_fitted.fib");
    CompareFibs(test, ref, "LocalMSE_fitted.fib");
    CompareImages(fitter->GetFittedImage(), "LocalMSE_fitted_image.nrrd");
    CompareImages(fitter->GetResidualImage(), "LocalMSE_residual_image.nrrd");
  }

  void Fit5()
  {
    omp_set_num_threads(1);
    fitter->SetLambda(0.1);
    fitter->SetFilterOutliers(false);
    fitter->SetRegularization(VnlCostFunction::GROUP_VARIANCE);
    fitter->Update();

    std::vector< mitk::FiberBundle::Pointer > output_tracts = fitter->GetTractograms();
    mitk::FiberBundle::Pointer test = mitk::FiberBundle::New();
    test = test->AddBundles(output_tracts);
    mitk::FiberBundle::Pointer ref = LoadFib("out/GroupMSE_fitted.fib");
    CompareFibs(test, ref, "GroupMSE_fitted.fib");
    CompareImages(fitter->GetFittedImage(), "GroupMSE_fitted_image.nrrd");
    CompareImages(fitter->GetResidualImage(), "GroupMSE_residual_image.nrrd");
  }

  void Fit6()
  {
    omp_set_num_threads(1);
    fitter->SetLambda(10);
    fitter->SetFilterOutliers(false);
    fitter->SetRegularization(VnlCostFunction::GROUP_LASSO);
    fitter->Update();

    std::vector< mitk::FiberBundle::Pointer > output_tracts = fitter->GetTractograms();
    mitk::FiberBundle::Pointer test = mitk::FiberBundle::New();
    test = test->AddBundles(output_tracts);
    mitk::FiberBundle::Pointer ref = LoadFib("out/GroupLasso_fitted.fib");
    CompareFibs(test, ref, "GroupLasso_fitted.fib");
    CompareImages(fitter->GetFittedImage(), "GroupLasso_fitted_image.nrrd");
    CompareImages(fitter->GetResidualImage(), "GroupLasso_residual_image.nrrd");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkFiberFit)
