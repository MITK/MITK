/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCommandLineParser.h"
#include <mitkConvert2Dto3DImageFilter.h>
#include "mitkIOUtil.h"

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

  std::string inputName = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outputName = us::any_cast<std::string>(parsedArgs["output"]);

  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(inputName);
  mitk::Convert2Dto3DImageFilter::Pointer multiFilter2 = mitk::Convert2Dto3DImageFilter::New();
  multiFilter2->SetInput(image);
  multiFilter2->Update();
  mitk::Image::Pointer image2 = multiFilter2->GetOutput();
  mitk::IOUtil::Save(image2, outputName);

  return EXIT_SUCCESS;
}
