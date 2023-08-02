/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommandLineParser.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

int main(int argc, char* argv[])
{

  mitkCommandLineParser parser;

  parser.setTitle("Contour Model Set to Image Converter");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("German Cancer Research Center (DKFZ)");
  parser.setArgumentPrefix("--","-");

  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file:", "Input contour model set", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("reference", "r", mitkCommandLineParser::Image, "Reference image", "Input reference image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::Image, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);

  auto args = parser.parseArguments(argc, argv);

  if (args.empty())
    return EXIT_FAILURE;

  try
  {
    auto inputFilename = us::any_cast<std::string>(args["input"]);
    auto referenceFilename = us::any_cast<std::string>(args["reference"]);
    auto outputFilename = us::any_cast<std::string>(args["output"]);

    auto input = mitk::IOUtil::Load<mitk::ContourModelSet>(inputFilename);
    auto referenceImage = mitk::IOUtil::Load<mitk::Image>(referenceFilename);

    auto filter = mitk::ContourModelSetToImageFilter::New();
    filter->SetInput(input);
    filter->SetImage(referenceImage);

    filter->Update();

    mitk::Image::Pointer output = filter->GetOutput();
    filter = nullptr;

    //auto labelSetImage = mitk::LabelSetImage::New();
    //labelSetImage->InitializeByLabeledImage(filter->GetOutput());

    //mitk::IOUtil::Save(labelSetImage, outputFilename);

    mitk::IOUtil::Save(output, outputFilename);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    return EXIT_FAILURE;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
