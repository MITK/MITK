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
#include <itkDwiGradientLengthCorrectionFilter.h>
#include <mitkDiffusionPropertyHelper.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("RoundBvalues");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Round b-values");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("to_nearest", "", mitkCommandLineParser::Int, "To nearest:", "integer", 1000);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string imageName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outImage = us::any_cast<std::string>(parsedArgs["o"]);

  int to_nearest = 1000;
  if (parsedArgs.count("to_nearest"))
    to_nearest = us::any_cast<int>(parsedArgs["to_nearest"]);

  try
  {
    typedef mitk::DiffusionPropertyHelper PropHelper;

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
    mitk::Image::Pointer in_image = mitk::IOUtil::Load<mitk::Image>(imageName, &functor);

    if (!PropHelper::IsDiffusionWeightedImage(in_image))
    {
      mitkThrow() << "Input is not a diffusion weighted image: " << imageName;
    }

    typedef itk::DwiGradientLengthCorrectionFilter FilterType;

    auto itkVectorImagePointer = PropHelper::GetItkVectorImage(in_image);

    FilterType::Pointer filter = FilterType::New();
    filter->SetRoundingValue(to_nearest);
    filter->SetReferenceBValue(PropHelper::GetReferenceBValue(in_image));
    filter->SetReferenceGradientDirectionContainer(PropHelper::GetGradientContainer(in_image));
    filter->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk( itkVectorImagePointer.GetPointer() );
    newImage->SetImportVolume( itkVectorImagePointer->GetBufferPointer(), 0, 0, mitk::Image::CopyMemory);
    itkVectorImagePointer->GetPixelContainer()->ContainerManageMemoryOff();

    PropHelper::CopyProperties(in_image, newImage, true);
    PropHelper::SetReferenceBValue(newImage, filter->GetNewBValue());
    PropHelper::SetGradientContainer(newImage, filter->GetOutputGradientDirectionContainer());
    PropHelper::InitializeImage(newImage);

    std::string ext = itksys::SystemTools::GetFilenameExtension(outImage);
    if (ext==".nii" || ext==".nii.gz")
      mitk::IOUtil::Save(newImage, "DWI_NIFTI", outImage);
    else
      mitk::IOUtil::Save(newImage, outImage);
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
