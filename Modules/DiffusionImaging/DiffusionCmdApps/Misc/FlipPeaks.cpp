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
#include <mitkPeakImage.h>
#include <itkFlipPeaksFilter.h>
#include <mitkImageToItk.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

/*!
\brief Copies transformation matrix of one image to another
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Flip Peaks");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Flips the peaks of the input peak image along the given dimensions.");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input", "input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("", "x", mitkCommandLineParser::Bool, "Flip x", "flip along x-axis");
  parser.addArgument("", "y", mitkCommandLineParser::Bool, "Flip y", "flip along y-axis");
  parser.addArgument("", "z", mitkCommandLineParser::Bool, "Flip z", "flip along z-axis");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string imageName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outImage = us::any_cast<std::string>(parsedArgs["o"]);

  bool x = false;
  if (parsedArgs.count("x"))
    x = us::any_cast<bool>(parsedArgs["x"]);

  bool y = false;
  if (parsedArgs.count("y"))
    y = us::any_cast<bool>(parsedArgs["y"]);

  bool z = false;
  if (parsedArgs.count("z"))
    z = us::any_cast<bool>(parsedArgs["z"]);

  try
  {
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image"}, {});
    mitk::PeakImage::Pointer image = mitk::IOUtil::Load<mitk::PeakImage>(imageName, &functor);

    typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(image);
    caster->Update();
    mitk::PeakImage::ItkPeakImageType::Pointer itkImg = caster->GetOutput();

    itk::FlipPeaksFilter< float >::Pointer flipper = itk::FlipPeaksFilter< float >::New();
    flipper->SetInput(itkImg);
    flipper->SetFlipX(x);
    flipper->SetFlipY(y);
    flipper->SetFlipZ(z);
    flipper->Update();

    mitk::Image::Pointer resultImage = dynamic_cast<mitk::Image*>(mitk::PeakImage::New().GetPointer());
    mitk::CastToMitkImage(flipper->GetOutput(), resultImage);
    resultImage->SetVolume(flipper->GetOutput()->GetBufferPointer());

    mitk::IOUtil::Save(resultImage, outImage);
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
