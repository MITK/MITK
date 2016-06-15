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

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"
#include <mitkBoundingObjectCutter.h>
#include <mitkCuboid.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Dicom Loader");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("reference", "r", mitkCommandLineParser::InputDirectory, "Input file:", "Input file",us::Any(),false);
  parser.addArgument("input", "i", mitkCommandLineParser::InputDirectory, "Input file:", "Input file",us::Any(),false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output file:", "Output file",us::Any(),false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string referenceName = us::any_cast<std::string>(parsedArgs["reference"]);
  std::string inputName = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outputName = us::any_cast<std::string>(parsedArgs["output"]);

  mitk::Image::Pointer imageToCrop = mitk::IOUtil::LoadImage(inputName);
  mitk::Image::Pointer referenceImage = mitk::IOUtil::LoadImage(referenceName);
  mitk::BoundingObjectCutter::Pointer cutter = mitk::BoundingObjectCutter::New();
  mitk::BoundingObject::Pointer boundingObject = (mitk::Cuboid::New()).GetPointer();
  boundingObject->FitGeometry(referenceImage->GetGeometry());

  cutter->SetBoundingObject(boundingObject);
  cutter->SetInput(imageToCrop);
  cutter->Update();
  mitk::IOUtil::SaveImage(cutter->GetOutput(), outputName);

  return EXIT_SUCCESS;
}