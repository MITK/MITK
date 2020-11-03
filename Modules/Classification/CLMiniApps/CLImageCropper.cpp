/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("reference", "r", mitkCommandLineParser::Directory, "Input file:", "Input file",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("input", "i", mitkCommandLineParser::Directory, "Input file:", "Input file",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file",us::Any(),false, false, false, mitkCommandLineParser::Output);

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

  mitk::Image::Pointer imageToCrop = mitk::IOUtil::Load<mitk::Image>(inputName);
  mitk::Image::Pointer referenceImage = mitk::IOUtil::Load<mitk::Image>(referenceName);
  mitk::BoundingObjectCutter::Pointer cutter = mitk::BoundingObjectCutter::New();
  mitk::BoundingObject::Pointer boundingObject = (mitk::Cuboid::New()).GetPointer();
  boundingObject->FitGeometry(referenceImage->GetGeometry());

  cutter->SetBoundingObject(boundingObject);
  cutter->SetInput(imageToCrop);
  cutter->Update();
  mitk::IOUtil::Save(cutter->GetOutput(), outputName);

  return EXIT_SUCCESS;
}
