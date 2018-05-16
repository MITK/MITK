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

typedef mitk::DiffusionPropertyHelper DPH;
/*!
\brief Copies transformation matrix of one image to another
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("DREG");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("TEMPORARY: Rigid registration of two images");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "f", mitkCommandLineParser::InputFile, "Fixed:", "fixed image", us::Any(), false);
  parser.addArgument("", "m", mitkCommandLineParser::InputFile, "Moving:", "moving image", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputFile, "Output:", "output image", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string f = us::any_cast<std::string>(parsedArgs["f"]);
  std::string m = us::any_cast<std::string>(parsedArgs["m"]);
  std::string o = us::any_cast<std::string>(parsedArgs["o"]);

  try
  {
    typedef itk::Image< float, 3 > ItkFloatImageType;

    mitk::Image::Pointer fixed = mitk::IOUtil::Load<mitk::Image>(f);
    mitk::Image::Pointer moving = mitk::IOUtil::Load<mitk::Image>(m);
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

    mitk::Image::Pointer registered_image = mitk::ImageMappingHelper::refineGeometry(moving, reg, true);

    if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(registered_image))
    {
      mitk::DiffusionPropertyHelper propertyHelper( registered_image );
      propertyHelper.InitializeImage();

      std::string file_extension = itksys::SystemTools::GetFilenameExtension(o);
      if (file_extension==".nii" || file_extension==".nii.gz")
        mitk::IOUtil::Save(registered_image, "application/vnd.mitk.nii.gz", o);
      else
        mitk::IOUtil::Save(registered_image, o);
    }
    else
      mitk::IOUtil::Save(registered_image, o);
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
