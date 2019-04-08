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

#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkTensorImage.h>
#include <mitkOdfImage.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include <itkStreamlineTrackingFilter.h>
#include <Algorithms/TrackingHandlers/mitkTrackingDataHandler.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerOdf.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerPeaks.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerTensor.h>
#include <itkDiffusionTensor3D.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkFiberBundle.h>
#include <omp.h>
#include <itksys/SystemTools.hxx>
#include <mitkEqual.h>
#include <mitkStreamlineTractographyParameters.h>

class mitkStreamlineTractographyTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkStreamlineTractographyTestSuite);
  MITK_TEST(Test_Peak1);
  MITK_TEST(Test_Peak2);
  MITK_TEST(Test_Tensor1);
  MITK_TEST(Test_Tensor2);
  MITK_TEST(Test_Tensor3);
  MITK_TEST(Test_Odf1);
  MITK_TEST(Test_Odf2);
  MITK_TEST(Test_Odf3);
  MITK_TEST(Test_Odf4);
  MITK_TEST(Test_Odf5);
  MITK_TEST(Test_Odf6);
  CPPUNIT_TEST_SUITE_END();

  typedef itk::VectorImage< short, 3>   ItkDwiType;

private:

public:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/

  typedef itk::Image<float, 3> ItkFloatImgType;

  mitk::TrackingHandlerOdf::ItkOdfImageType::Pointer itk_odf_image;
  mitk::TrackingHandlerTensor::ItkTensorImageType::ConstPointer itk_tensor_image;
  mitk::TrackingHandlerPeaks::PeakImgType::Pointer itk_peak_image;
  ItkFloatImgType::Pointer itk_seed_image;
  ItkFloatImgType::Pointer itk_mask_image;
  ItkFloatImgType::Pointer itk_gfa_image;

  float gfa_threshold;
  float odf_threshold;
  float peak_threshold;
  std::shared_ptr<mitk::StreamlineTractographyParameters> params;

  itk::StreamlineTrackingFilter::Pointer tracker;

  void setUp() override
  {
    omp_set_num_threads(1);

    gfa_threshold = 0.2f;
    odf_threshold = 0.1f;
    peak_threshold = 0.1f;

    mitk::Image::Pointer odf_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/qball_image.qbi"));
    mitk::Image::Pointer tensor_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/tensor_image.dti"));
    mitk::Image::Pointer peak_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/qball_peak_image.nii.gz"));
    mitk::Image::Pointer seed_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/seed_image.nii.gz"));
    mitk::Image::Pointer mask_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/mask_image.nii.gz"));
    mitk::Image::Pointer gfa_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/gfa_image.nii.gz"));

    params = std::make_shared<mitk::StreamlineTractographyParameters>();
    params->m_FixRandomSeed = true;
    params->m_InterpolateRoiImages = false;

    {
      typedef mitk::ImageToItk< mitk::TrackingHandlerPeaks::PeakImgType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(peak_image);
      caster->Update();
      itk_peak_image = caster->GetOutput();
    }

    {
      typedef mitk::ImageToItk< mitk::TrackingHandlerTensor::ItkTensorImageType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(tensor_image);
      caster->Update();
      itk_tensor_image = caster->GetOutput();
    }

    {
      typedef mitk::ImageToItk< mitk::TrackingHandlerOdf::ItkOdfImageType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(odf_image);
      caster->Update();
      itk_odf_image = caster->GetOutput();
    }

    itk_gfa_image = ItkFloatImgType::New();
    mitk::CastToItkImage(gfa_image, itk_gfa_image);

    itk_seed_image = ItkFloatImgType::New();
    mitk::CastToItkImage(seed_image, itk_seed_image);

    itk_mask_image = ItkFloatImgType::New();
    mitk::CastToItkImage(mask_image, itk_mask_image);
  }

  mitk::FiberBundle::Pointer LoadReferenceFib(std::string filename)
  {
    mitk::FiberBundle::Pointer fib = nullptr;

    if (itksys::SystemTools::FileExists(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/ReferenceFibs/" + filename)))
    {
      mitk::BaseData::Pointer baseData = mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/ReferenceFibs/" + filename)).at(0);
      fib = dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
    }
    return fib;
  }

  mitk::Image::Pointer LoadReferenceImage(std::string filename)
  {
    mitk::Image::Pointer img = nullptr;

    if (itksys::SystemTools::FileExists(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/ReferenceFibs/" + filename)))
    {
      img = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/StreamlineTractography/ReferenceFibs/" + filename));
    }
    return img;
  }

  void SetupTracker(mitk::TrackingDataHandler* handler)
  {

    tracker = itk::StreamlineTrackingFilter::New();

//    tracker->SetInterpolateMasks(false);
//    tracker->SetNumberOfSamples(0);
//    tracker->SetAngularThreshold(-1);
    tracker->SetMaskImage(itk_mask_image);
    tracker->SetSeedImage(itk_seed_image);
    tracker->SetStoppingRegions(nullptr);
//    tracker->SetSeedsPerVoxel(1);
//    tracker->SetStepSize(0.5);
//    tracker->SetSamplingDistance(0.25);
//    tracker->SetUseStopVotes(true);
//    tracker->SetOnlyForwardSamples(true);
//    tracker->SetMinTractLength(20);
//    tracker->SetMaxNumTracts(-1);
    tracker->SetTrackingHandler(handler);
//    tracker->SetUseOutputProbabilityMap(false);
    tracker->SetParameters(params);
  }

  void tearDown() override
  {

  }

  void CheckFibResult(std::string ref_file, mitk::FiberBundle::Pointer test_fib)
  {
    mitk::FiberBundle::Pointer ref = LoadReferenceFib(ref_file);
    if (ref.IsNull())
    {
      mitk::IOUtil::Save(test_fib, mitk::IOUtil::GetTempPath()+ref_file);
      CPPUNIT_FAIL("Reference file not found. Saving test file to " + mitk::IOUtil::GetTempPath() + ref_file);
    }
    else
    {
      bool is_equal = ref->Equals(test_fib);
      if (!is_equal)
      {
        mitk::IOUtil::Save(test_fib, mitk::IOUtil::GetTempPath()+ref_file);
        CPPUNIT_FAIL("Tractograms are not equal! Saving test file to " + mitk::IOUtil::GetTempPath() + ref_file);
      }
    }
  }

  void CheckImageResult(std::string ref_file, mitk::Image::Pointer test_img)
  {
    mitk::Image::Pointer ref = LoadReferenceImage(ref_file);
    if (ref.IsNull())
    {
      mitk::IOUtil::Save(test_img, mitk::IOUtil::GetTempPath()+ref_file);
      CPPUNIT_FAIL("Reference file not found. Saving test file to " + mitk::IOUtil::GetTempPath() + ref_file);
    }
    else
    {
      MITK_ASSERT_EQUAL(test_img, ref, "Images should be equal");
    }
  }

  void Test_Peak1()
  {
    mitk::TrackingHandlerPeaks* handler = new mitk::TrackingHandlerPeaks();
    handler->SetPeakImage(itk_peak_image);
    params->m_Cutoff = peak_threshold;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Peak1.fib", outFib);

    delete handler;
  }

  void Test_Peak2()
  {
    mitk::TrackingHandlerPeaks* handler = new mitk::TrackingHandlerPeaks();
    handler->SetPeakImage(itk_peak_image);
    params->m_Cutoff = peak_threshold;
    params->m_InterpolateTractographyData = false;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Peak2.fib", outFib);

    delete handler;
  }

  void Test_Tensor1()
  {
    mitk::TrackingHandlerTensor* handler = new mitk::TrackingHandlerTensor();
    handler->SetTensorImage(itk_tensor_image);
    params->m_Cutoff = gfa_threshold;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Tensor1.fib", outFib);

    delete handler;
  }

  void Test_Tensor2()
  {
    mitk::TrackingHandlerTensor* handler = new mitk::TrackingHandlerTensor();
    handler->SetTensorImage(itk_tensor_image);
    params->m_Cutoff = gfa_threshold;
    params->m_InterpolateTractographyData = false;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Tensor2.fib", outFib);

    delete handler;
  }

  void Test_Tensor3()
  {
    mitk::TrackingHandlerTensor* handler = new mitk::TrackingHandlerTensor();
    handler->SetTensorImage(itk_tensor_image);
    params->m_Cutoff = gfa_threshold;
    params->m_InterpolateTractographyData = false;
    params->m_F = 0;
    params->m_G = 1;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Tensor3.fib", outFib);

    delete handler;
  }

  void Test_Odf1()
  {
    mitk::TrackingHandlerOdf* handler = new mitk::TrackingHandlerOdf();
    handler->SetOdfImage(itk_odf_image);

    params->m_Cutoff = gfa_threshold;
    params->m_OdfCutoff = 0;
    params->m_SharpenOdfs = true;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Odf1.fib", outFib);

    delete handler;
  }

  void Test_Odf2()
  {
    mitk::TrackingHandlerOdf* handler = new mitk::TrackingHandlerOdf();
    handler->SetOdfImage(itk_odf_image);

    params->m_Cutoff = gfa_threshold;
    params->m_OdfCutoff = 0;
    params->m_SharpenOdfs = false;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Odf2.fib", outFib);

    delete handler;
  }


  void Test_Odf3()
  {
    mitk::TrackingHandlerOdf* handler = new mitk::TrackingHandlerOdf();
    handler->SetOdfImage(itk_odf_image);

    params->m_Cutoff = gfa_threshold;
    params->m_OdfCutoff = 0;
    params->m_SharpenOdfs = true;
    params->m_InterpolateTractographyData = false;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Odf3.fib", outFib);

    delete handler;
  }

  void Test_Odf4()
  {
    mitk::TrackingHandlerOdf* handler = new mitk::TrackingHandlerOdf();
    handler->SetOdfImage(itk_odf_image);

    params->m_Cutoff = gfa_threshold;
    params->m_OdfCutoff = 0;
    params->m_SharpenOdfs = true;
    params->m_SeedsPerVoxel = 3;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Odf4.fib", outFib);

    delete handler;
  }

  void Test_Odf5()
  {
    mitk::TrackingHandlerOdf* handler = new mitk::TrackingHandlerOdf();
    handler->SetOdfImage(itk_odf_image);

    params->m_Cutoff = gfa_threshold;
    params->m_OdfCutoff = 0;
    params->m_SharpenOdfs = true;
    params->m_SeedsPerVoxel = 3;
    params->m_Mode = mitk::TrackingDataHandler::MODE::PROBABILISTIC;

    SetupTracker(handler);
    tracker->Update();

    vtkSmartPointer< vtkPolyData > poly = tracker->GetFiberPolyData();
    mitk::FiberBundle::Pointer outFib = mitk::FiberBundle::New(poly);

    CheckFibResult("Test_Odf5.fib", outFib);

    delete handler;
  }

  void Test_Odf6()
  {
    mitk::TrackingHandlerOdf* handler = new mitk::TrackingHandlerOdf();
    handler->SetOdfImage(itk_odf_image);

    params->m_Cutoff = gfa_threshold;
    params->m_OdfCutoff = 0;
    params->m_SharpenOdfs = true;
    params->m_SeedsPerVoxel = 10;
    params->m_Mode = mitk::TrackingDataHandler::MODE::PROBABILISTIC;
    params->m_OutputProbMap = true;

    SetupTracker(handler);
    tracker->Update();

    itk::StreamlineTrackingFilter::ItkDoubleImgType::Pointer outImg = tracker->GetOutputProbabilityMap();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    mitk::IOUtil::Save(img, mitk::IOUtil::GetTempPath()+"Test_Odf6.nrrd");
    CheckImageResult("Test_Odf6.nrrd", img);

    delete handler;
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkStreamlineTractography)
