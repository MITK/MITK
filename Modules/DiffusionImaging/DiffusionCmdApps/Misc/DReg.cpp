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
#include <mitkMultiModalRigidDefaultRegistrationAlgorithm.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkAlgorithmHelper.h>
#include <itkExtractDwiChannelFilter.h>
#include <mitkRegistrationHelper.h>
#include <mitkImageMappingHelper.h>
#include <itksys/SystemTools.hxx>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itkComposeImageFilter.h>
#include <mitkITKImageImport.h>
#include <mitkDiffusionImageCorrectionFilter.h>
#include <mitkDiffusionDataIOHelper.h>

typedef mitk::DiffusionPropertyHelper DPH;


mitk::Image::Pointer apply_transform(mitk::Image::Pointer moving, mitk::Image::Pointer fixed_single, mitk::MAPRegistrationWrapper::Pointer reg, bool resample)
{
  mitk::Image::Pointer registered_image;
  if (!resample)
  {
    registered_image = mitk::ImageMappingHelper::refineGeometry(moving, reg, true);
  }
  else
  {
    if (!mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(moving))
    {
      registered_image = mitk::ImageMappingHelper::map(moving, reg, false, 0, fixed_single->GetGeometry(), false, 0, mitk::ImageMappingInterpolator::BSpline_3);
    }
    else
    {
      typedef itk::Image<mitk::DiffusionPropertyHelper::DiffusionPixelType, 3> ITKDiffusionVolumeType;
      typedef itk::ComposeImageFilter < ITKDiffusionVolumeType > ComposeFilterType;
      auto composer = ComposeFilterType::New();

      auto itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(moving);
      for (unsigned int i=0; i<itkVectorImagePointer->GetVectorLength(); ++i)
      {
        itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
        filter->SetInput( itkVectorImagePointer);
        filter->SetChannelIndex(i);
        filter->Update();

        mitk::Image::Pointer gradientVolume = mitk::Image::New();
        gradientVolume->InitializeByItk( filter->GetOutput() );
        gradientVolume->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

        mitk::Image::Pointer registered_mitk_image = mitk::ImageMappingHelper::map(gradientVolume, reg, false, 0, fixed_single->GetGeometry(), false, 0, mitk::ImageMappingInterpolator::BSpline_3);

        auto registered_itk_image = ITKDiffusionVolumeType::New();
        mitk::CastToItkImage(registered_mitk_image, registered_itk_image);
        composer->SetInput(i, registered_itk_image);
      }
      composer->Update();

      registered_image = mitk::GrabItkImageMemory( composer->GetOutput() );
      mitk::DiffusionPropertyHelper::CopyProperties(moving, registered_image, true);

      typedef mitk::DiffusionImageCorrectionFilter CorrectionFilterType;
      CorrectionFilterType::Pointer corrector = CorrectionFilterType::New();
      corrector->SetImage( registered_image );
      corrector->CorrectDirections( mitk::MITKRegistrationHelper::getAffineMatrix(reg, false)->GetMatrix().GetVnlMatrix() );
    }
  }

  return registered_image;
}


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("DREG");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("TEMPORARY: Rigid registration of two images");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "f", mitkCommandLineParser::String, "Fixed:", "fixed image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "m", mitkCommandLineParser::String, "Moving:", "moving image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("resample", "", mitkCommandLineParser::Bool, "Resample:", "resample moving image", false);
  parser.addArgument("coreg", "", mitkCommandLineParser::StringList, "", "additionally apply transform to these images", us::Any(), true, false, false, mitkCommandLineParser::Input);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string f = us::any_cast<std::string>(parsedArgs["f"]);
  std::string m = us::any_cast<std::string>(parsedArgs["m"]);
  std::string o = us::any_cast<std::string>(parsedArgs["o"]);

  bool resample = false;
  if (parsedArgs.count("resample"))
    resample = true;

  mitkCommandLineParser::StringContainerType coreg;
  if (parsedArgs.count("coreg"))
    coreg = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["coreg"]);

  try
  {
    typedef itk::Image< float, 3 > ItkFloatImageType;

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
    mitk::Image::Pointer fixed = mitk::IOUtil::Load<mitk::Image>(f, &functor);
    mitk::Image::Pointer moving = mitk::IOUtil::Load<mitk::Image>(m, &functor);

    mitk::Image::Pointer fixed_single = fixed;
    mitk::Image::Pointer moving_single = moving;

    mitk::MultiModalRigidDefaultRegistrationAlgorithm< ItkFloatImageType >::Pointer algo = mitk::MultiModalRigidDefaultRegistrationAlgorithm< ItkFloatImageType >::New();
    mitk::MITKAlgorithmHelper helper(algo);

    if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(fixed))
    {
      DPH::ImageType::Pointer itkVectorImagePointer = DPH::ImageType::New();
      mitk::CastToItkImage(fixed, itkVectorImagePointer);

      itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
      filter->SetInput( itkVectorImagePointer);
      filter->SetChannelIndex(0);
      filter->Update();

      fixed_single = mitk::Image::New();
      fixed_single->InitializeByItk( filter->GetOutput() );
      fixed_single->SetImportChannel( filter->GetOutput()->GetBufferPointer() );
    }

    if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(moving))
    {
      DPH::ImageType::Pointer itkVectorImagePointer = DPH::ImageType::New();
      mitk::CastToItkImage(moving, itkVectorImagePointer);

      itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
      filter->SetInput( itkVectorImagePointer);
      filter->SetChannelIndex(0);
      filter->Update();

      moving_single = mitk::Image::New();
      moving_single->InitializeByItk( filter->GetOutput() );
      moving_single->SetImportChannel( filter->GetOutput()->GetBufferPointer() );
    }

    helper.SetData(moving_single, fixed_single);
    mitk::MAPRegistrationWrapper::Pointer reg = helper.GetMITKRegistrationWrapper();

    mitk::Image::Pointer registered_image = apply_transform(moving, fixed_single, reg, resample);
    if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(registered_image))
    {
      mitk::DiffusionPropertyHelper::InitializeImage( registered_image );

      std::string file_extension = itksys::SystemTools::GetFilenameExtension(o);
      if (file_extension==".nii" || file_extension==".nii.gz")
        mitk::IOUtil::Save(registered_image, "DWI_NIFTI", o);
      else
        mitk::IOUtil::Save(registered_image, o);
    }
    else
      mitk::IOUtil::Save(registered_image, o);

    std::string path = ist::GetFilenamePath(o) + "/";
    std::vector< std::string > file_names;
    auto coreg_images = mitk::DiffusionDataIOHelper::load_mitk_images(coreg, &file_names);
    for (unsigned int i=0; i<coreg_images.size(); ++i)
    {
      std::string ext = ist::GetFilenameExtension(file_names.at(i));
      std::string out_name = path + ist::GetFilenameWithoutExtension(file_names.at(i)) + "_registered" + ext;

      registered_image = apply_transform(coreg_images.at(i), fixed_single, reg, resample);
      if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(registered_image))
      {
        mitk::DiffusionPropertyHelper::InitializeImage( registered_image );

        if (ext==".nii" || ext==".nii.gz")
          mitk::IOUtil::Save(registered_image, "DWI_NIFTI", out_name);
        else
          mitk::IOUtil::Save(registered_image, out_name);
      }
      else
        mitk::IOUtil::Save(registered_image, out_name);
    }
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
