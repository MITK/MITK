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

/*!
\brief Copies transformation matrix of one image to another
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("DIMP");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("TEMPORARY: Converts DICOM to other image types");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("in", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output:", "output image", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string imageName = us::any_cast<std::string>(parsedArgs["in"]);
  std::string outImage = us::any_cast<std::string>(parsedArgs["out"]);

  try
  {
    mitk::Image::Pointer source = mitk::IOUtil::Load<mitk::Image>(imageName);
    mitk::IOUtil::Save(source, outImage);
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
