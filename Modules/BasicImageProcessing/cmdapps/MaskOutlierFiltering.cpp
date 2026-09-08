/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkProperties.h>

#include <mitkCommandLineParser.h>
#include <mitkIOUtil.h>

#include <mitkMaskCleaningOperation.h>


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Mask Outlier Filtering");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("Removes all voxels from a mask whose image intensity lies outside three standard deviations around the mean of the masked intensities.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("image", "i", mitkCommandLineParser::File, "Input image:", "Input Image",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::File, "Input mask:", "Input Mask", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Filtered output mask", us::Any(), false, false, false, mitkCommandLineParser::Output);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  std::string inputFilename = us::any_cast<std::string>(parsedArgs["image"]);
  std::string maskFilename = us::any_cast<std::string>(parsedArgs["mask"]);
  std::string outputFilename = us::any_cast<std::string>(parsedArgs["output"]);

  try
  {
    auto nodes = mitk::IOUtil::Load(inputFilename);
    if (nodes.size() == 0)
    {
      MITK_ERROR << "No Image Loaded";
      return EXIT_FAILURE;
    }
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(nodes[0].GetPointer());

    if (image.IsNull())
    {
      MITK_ERROR << "Loaded data (image) is not of type image";
      return EXIT_FAILURE;
    }


    auto maskNodes = mitk::IOUtil::Load(maskFilename);
    if (maskNodes.size() == 0)
    {
      MITK_ERROR << "No Mask Loaded";
      return EXIT_FAILURE;
    }
    mitk::Image::Pointer mask = dynamic_cast<mitk::Image*>(maskNodes[0].GetPointer());

    if (image.IsNull())
    {
      MITK_ERROR << "Loaded data (mask) is not of type image";
      return EXIT_FAILURE;
    }

    mitk::Image::Pointer tmpImage = mitk::MaskCleaningOperation::MaskOutlierFiltering(image, mask);

    mitk::IOUtil::Save(tmpImage, outputFilename);

    return EXIT_SUCCESS;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_ERROR << "Unexpected error encountered.";
    return EXIT_FAILURE;
  }
}
