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
#include <itkExtractDwiChannelFilter.h>

using namespace mitk;
using namespace std;

/*!
\brief Copies transformation matrix of one image to another
*/
int main(int argc, char* argv[])
{
  typedef itk::VectorImage< short, 3 >                                    ItkDwiType;

  mitkCommandLineParser parser;

  parser.setTitle("Extract all gradients");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Extract all gradients from an diffusion image");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("in", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(), false);
  //parser.addArgument("extension", "e", mitkCommandLineParser::String, "File Extension:", "Extension of the output file", us::Any(), false);
  //parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output:", "output image", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    return EXIT_FAILURE;

  MITK_INFO << "Extract parameter";
  // mandatory arguments
  /*string inputName = us::any_cast<string>(parsedArgs["in"]);
  string extensionName = us::any_cast<string>(parsedArgs["extension"]);
  string ouputName = us::any_cast<string>(parsedArgs["out"]);*/
  string in = us::any_cast<string>(parsedArgs["in"]);
  string inputName = "E:\\Kollektive\\R02-Lebertumore-Diffusion\\01-Extrahierte-Daten\\" + in + "\\" + in + "-DWI.dwi";
  string extensionName = ".nrrd";
  string ouputName = "E:\\Kollektive\\R02-Lebertumore-Diffusion\\01-Extrahierte-Daten\\" + in + "\\" + in + "-";

  MITK_INFO << "Load Image: ";
  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(inputName);

  //bool isDiffusionImage(mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image));
  //if (!isDiffusionImage)
  //{
  //  MITK_INFO << "Input file is not of type diffusion image";
  //  return;
  //}

  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);

  unsigned int channel = 0;
  for (unsigned int channel = 0; channel < itkVectorImagePointer->GetVectorLength(); ++channel)
  {
    itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
    filter->SetInput(itkVectorImagePointer);
    filter->SetChannelIndex(channel);
    filter->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk(filter->GetOutput());
    newImage->SetImportChannel(filter->GetOutput()->GetBufferPointer());
    mitk::IOUtil::SaveImage(newImage, ouputName + to_string(channel) + extensionName);
  }
    return EXIT_SUCCESS;
}
