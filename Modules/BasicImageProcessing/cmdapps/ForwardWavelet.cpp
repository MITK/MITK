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

#include <mitkTransformationOperation.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Forward Wavelet");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("Computes the forward wavelet transformation of an image and writes one image per sub-band.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("image", "i", mitkCommandLineParser::File, "Input image:", "Image the wavelet decomposition is computed of", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output prefix:", "Prefix of the output files, including the path", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output-extension", "e", mitkCommandLineParser::File, "Output extension:", "File extension of the output files, including the dot, e.g. .nrrd", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("number-of-levels", "levels", mitkCommandLineParser::Int, "Numbers of pyramid levels", "Number of decomposition levels", us::Any(), false);
  parser.addArgument("number-of-bands", "bands", mitkCommandLineParser::Int, "Number of bands", "Number of high-pass sub-bands per level", us::Any(), false);

  parser.addArgument("wavelet", "w", mitkCommandLineParser::Int, "Wavelet family", "0: Shannon, 1: Simoncelli, 2: Vow, 3: Held", us::Any(), false);
  parser.addArgument("border-condition", "border", mitkCommandLineParser::Int, "Border condition", "0: Constant, 1: Periodic, 2: Zero Flux Neumann", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
    return EXIT_FAILURE;

  std::string inputFilename = us::any_cast<std::string>(parsedArgs["image"]);
  std::string outputFilename = us::any_cast<std::string>(parsedArgs["output"]);
  std::string outputExtension = us::any_cast<std::string>(parsedArgs["output-extension"]);

  try
  {
    auto nodes = mitk::IOUtil::Load(inputFilename);
    if (nodes.empty())
    {
      MITK_ERROR << "No data found in " << inputFilename;
      return EXIT_FAILURE;
    }
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(nodes[0].GetPointer());

    if (image.IsNull())
    {
      MITK_ERROR << "Loaded data is not an image: " << inputFilename;
      return EXIT_FAILURE;
    }

    int levels = us::any_cast<int>(parsedArgs["number-of-levels"]);
    int bands = us::any_cast<int>(parsedArgs["number-of-bands"]);

    mitk::BorderCondition condition = mitk::BorderCondition::Constant;
    mitk::WaveletType waveletType = mitk::WaveletType::Held;
    switch (us::any_cast<int>(parsedArgs["wavelet"]))
    {
    case 0:
      waveletType = mitk::WaveletType::Shannon;
      break;
    case 1:
      waveletType = mitk::WaveletType::Simoncelli;
      break;
    case 2:
      waveletType = mitk::WaveletType::Vow;
      break;
    case 3:
      waveletType = mitk::WaveletType::Held;
      break;
    default:
      waveletType = mitk::WaveletType::Shannon;
      break;
    }
    switch (us::any_cast<int>(parsedArgs["border-condition"]))
    {
    case 0:
      condition = mitk::BorderCondition::Constant;
      break;
    case 1:
      condition = mitk::BorderCondition::Periodic;
      break;
    case 2:
      condition = mitk::BorderCondition::ZeroFluxNeumann;
      break;
    default:
      condition = mitk::BorderCondition::Constant;
      break;
    }

    std::vector<mitk::Image::Pointer> results = mitk::TransformationOperation::WaveletForward(image, levels, bands, condition, waveletType);
    unsigned int level = 0;
    for (auto result : results)
    {
      std::string name = outputFilename + us::Any(level).ToString() + outputExtension;
      MITK_INFO << "Saving to " << name;
      mitk::IOUtil::Save(result, name);
      ++level;
    }

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
