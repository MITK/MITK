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
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itksys/SystemTools.hxx>
#include <itkTotalVariationDenoisingImageFilter.h>
#include <itkNonLocalMeansDenoisingFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkPatchBasedDenoisingImageFilter.h>
#include <itkVectorImageToImageFilter.h>
#include <itkComposeImageFilter.h>
#include <mitkDiffusionPropertyHelper.h>
#include <itkGaussianRandomSpatialNeighborSubsampler.h>
#include <mitkITKImageImport.h>
#include <mitkProperties.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("DmriDenoising");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("dMRI denoising tool");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("1. Mandatory arguments:");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("type", "", mitkCommandLineParser::Int, "Type:", "0 (TotalVariation), 1 (Gauss), 2 (NLM)", 0);
  parser.endGroup();

  parser.beginGroup("2. Total variation parameters:");
  parser.addArgument("tv_iterations", "", mitkCommandLineParser::Int, "Iterations:", "", 1);
  parser.addArgument("lambda", "", mitkCommandLineParser::Float, "Lambda:", "", 0.1);
  parser.endGroup();

  parser.beginGroup("3. Gauss parameters:");
  parser.addArgument("variance", "", mitkCommandLineParser::Float, "Variance:", "", 1.0);
  parser.endGroup();

  parser.beginGroup("4. NLM parameters:");
  parser.addArgument("nlm_iterations", "", mitkCommandLineParser::Int, "Iterations:", "", 4);
  parser.addArgument("sampling_radius", "", mitkCommandLineParser::Int, "Sampling radius:", "", 4);
  parser.addArgument("patch_radius", "", mitkCommandLineParser::Int, "Patch radius:", "", 1);
  parser.addArgument("num_patches", "", mitkCommandLineParser::Int, "Num. patches:", "", 10);
  parser.endGroup();

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string imageName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outImage = us::any_cast<std::string>(parsedArgs["o"]);

  int type = 0;
  if (parsedArgs.count("type"))
    type = us::any_cast<int>(parsedArgs["type"]);

  int tv_iterations = 1;
  if (parsedArgs.count("tv_iterations"))
    tv_iterations = us::any_cast<int>(parsedArgs["tv_iterations"]);

  float lambda = 0.1;
  if (parsedArgs.count("lambda"))
    lambda = us::any_cast<float>(parsedArgs["lambda"]);

  float variance = 1.0;
  if (parsedArgs.count("variance"))
    variance = us::any_cast<float>(parsedArgs["variance"]);

  int nlm_iterations = 4;
  if (parsedArgs.count("nlm_iterations"))
    nlm_iterations = us::any_cast<int>(parsedArgs["nlm_iterations"]);

  int sampling_radius = 4;
  if (parsedArgs.count("sampling_radius"))
    sampling_radius = us::any_cast<int>(parsedArgs["sampling_radius"]);

  int patch_radius = 1;
  if (parsedArgs.count("patch_radius"))
    patch_radius = us::any_cast<int>(parsedArgs["patch_radius"]);

  int num_patches = 10;
  if (parsedArgs.count("num_patches"))
    num_patches = us::any_cast<int>(parsedArgs["num_patches"]);

  try
  {
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
    mitk::Image::Pointer input_image = mitk::IOUtil::Load<mitk::Image>(imageName, &functor);

    typedef short                                                                         DiffusionPixelType;
    typedef itk::VectorImage<DiffusionPixelType, 3>                                       DwiImageType;
    typedef itk::Image<DiffusionPixelType, 3>                                             DwiVolumeType;
    typedef itk::DiscreteGaussianImageFilter < DwiVolumeType, DwiVolumeType >             GaussianFilterType;
    typedef itk::PatchBasedDenoisingImageFilter < DwiVolumeType, DwiVolumeType >          NlmFilterType;
    typedef itk::VectorImageToImageFilter < DiffusionPixelType >                          ExtractFilterType;
    typedef itk::ComposeImageFilter < itk::Image<DiffusionPixelType, 3> >                 ComposeFilterType;

    if (!mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(input_image))
      mitkThrow() << "Input is not a diffusion-weighted image!";

    DwiImageType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(input_image);

    mitk::Image::Pointer denoised_image = nullptr;

    switch (type)
    {
    case 0:
    {
      ComposeFilterType::Pointer composer = ComposeFilterType::New();
      for (unsigned int i=0; i<itkVectorImagePointer->GetVectorLength(); ++i)
      {
        ExtractFilterType::Pointer extractor = ExtractFilterType::New();
        extractor->SetInput( itkVectorImagePointer );
        extractor->SetIndex( i );
        extractor->Update();
        DwiVolumeType::Pointer gradient_volume = extractor->GetOutput();
        itk::TotalVariationDenoisingImageFilter< DwiVolumeType, DwiVolumeType >::Pointer filter = itk::TotalVariationDenoisingImageFilter< DwiVolumeType, DwiVolumeType >::New();
        filter->SetInput(gradient_volume);
        filter->SetLambda(lambda);
        filter->SetNumberIterations(tv_iterations);
        filter->Update();
        composer->SetInput(i, filter->GetOutput());
      }
      composer->Update();
      denoised_image = mitk::GrabItkImageMemory(composer->GetOutput());
      break;
    }
    case 1:
    {
      ExtractFilterType::Pointer extractor = ExtractFilterType::New();
      extractor->SetInput( itkVectorImagePointer );
      ComposeFilterType::Pointer composer = ComposeFilterType::New();
      for (unsigned int i = 0; i < itkVectorImagePointer->GetVectorLength(); ++i)
      {
        extractor->SetIndex(i);
        extractor->Update();
        GaussianFilterType::Pointer filter = GaussianFilterType::New();
        filter->SetVariance(variance);
        filter->SetInput(extractor->GetOutput());
        filter->Update();
        composer->SetInput(i, filter->GetOutput());
      }
      composer->Update();
      denoised_image = mitk::GrabItkImageMemory(composer->GetOutput());
      break;
    }
    case 2:
    {
      typedef itk::Statistics::GaussianRandomSpatialNeighborSubsampler< NlmFilterType::PatchSampleType, DwiVolumeType::RegionType > SamplerType;
      // sampling the image to find similar patches
      SamplerType::Pointer sampler = SamplerType::New();
      sampler->SetRadius( sampling_radius );
      sampler->SetVariance( sampling_radius*sampling_radius );
      sampler->SetNumberOfResultsRequested( num_patches );

      MITK_INFO << "Starting NLM denoising";
      ExtractFilterType::Pointer extractor = ExtractFilterType::New();
      extractor->SetInput( itkVectorImagePointer );
      ComposeFilterType::Pointer composer = ComposeFilterType::New();
      for (unsigned int i = 0; i < itkVectorImagePointer->GetVectorLength(); ++i)
      {
        extractor->SetIndex(i);
        extractor->Update();
        NlmFilterType::Pointer filter = NlmFilterType::New();
        filter->SetInput(extractor->GetOutput());
        filter->SetPatchRadius(patch_radius);
        filter->SetNoiseModel(NlmFilterType::RICIAN);
        filter->UseSmoothDiscPatchWeightsOn();
        filter->UseFastTensorComputationsOn();

        filter->SetNumberOfIterations(nlm_iterations);
        filter->SetSmoothingWeight( 1 );
        filter->SetKernelBandwidthEstimation(true);

        filter->SetSampler( sampler );
        filter->Update();
        composer->SetInput(i, filter->GetOutput());
        MITK_INFO << "Gradient " << i << " finished";
      }
      composer->Update();
      denoised_image = mitk::GrabItkImageMemory(composer->GetOutput());
      break;
    }
    }

    mitk::DiffusionPropertyHelper::SetGradientContainer(denoised_image, mitk::DiffusionPropertyHelper::GetGradientContainer(input_image));
    mitk::DiffusionPropertyHelper::SetReferenceBValue(denoised_image, mitk::DiffusionPropertyHelper::GetReferenceBValue(input_image));
    mitk::DiffusionPropertyHelper::InitializeImage( denoised_image );

    std::string ext = itksys::SystemTools::GetFilenameExtension(outImage);
    if (ext==".nii" || ext==".nii.gz")
      mitk::IOUtil::Save(denoised_image, "DWI_NIFTI", outImage);
    else
      mitk::IOUtil::Save(denoised_image, outImage);
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
