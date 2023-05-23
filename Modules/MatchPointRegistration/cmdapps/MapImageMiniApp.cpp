/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// std includes
#include <string>
#include <numeric>

// itk includes
#include "itksys/SystemTools.hxx"

// CTK includes
#include "mitkCommandLineParser.h"

// MITK includes
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkMAPRegistrationWrapper.h>
#include <mitkMAPAlgorithmHelper.h>
#include <mitkImageStitchingHelper.h>

std::string inFileName = "";
std::string regFileName = "";
std::string outFileName = "";
std::string refGeometryFileName = "";

mitk::Image::ConstPointer inputImage;
mitk::MAPRegistrationWrapper::ConstPointer registration;
mitk::BaseGeometry::Pointer refGeometry;
mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear;
double paddingValue = 0;
std::vector<unsigned int> superSamplingFactors;

void setupParser(mitkCommandLineParser& parser)
{
  // set general information about your MiniApp
  parser.setCategory("Registration Tools");
  parser.setTitle("Map Image");
  parser.setDescription("MiniApp that allows to map a image into a given output geometry by using a given registration.");
  parser.setContributor("MIC, German Cancer Research Center (DKFZ)");
  //! [create parser]

  //! [add arguments]
  // how should arguments be prefixed
  parser.setArgumentPrefix("--", "-");
  // add each argument, unless specified otherwise each argument is optional
  // see mitkCommandLineParser::addArgument for more information
  parser.beginGroup("Required I/O parameters");
  parser.addArgument(
    "input", "i", mitkCommandLineParser::File, "Input image", "Path to the input images that should be mapped", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output",
    "o",
    mitkCommandLineParser::File,
    "Output file path",
    "Path to the maped image.",
    us::Any(),
    false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "registration", "r", mitkCommandLineParser::File, "Registration filee", "Path to the registration that should be used. If no registration is specified, an identity transform is assumed.", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("template",
    "t",
    mitkCommandLineParser::File,
    "Output template image.",
    "File path to an image that serves as template for the output geometry. If no template is specified, the geometry of the input image will be used.",
    us::Any(),
    false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "registrations", "r", mitkCommandLineParser::StringList, "Registration files", "Pathes to the registrations that should be used to map the input images. If this parameter is not set, identity transforms are assumed. If this parameter is set, it must have the same number of entries then the parameter inputs. If you want to use and identity transform for a specific input, specify an empty string. The application assumes that inputs and registrations have the same order, so the n-th input should use thr n-th registration.", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("interpolator", "n", mitkCommandLineParser::Int, "Interpolator type", "Interpolator used for mapping the images. Default: 2; allowed values: 1: Nearest Neighbour, 2: Linear, 3: BSpline 3, 4: WSinc Hamming, 5: WSinc Welch", us::Any(2), true);
  parser.addArgument("padding", "p", mitkCommandLineParser::Float, "Padding value", "Value used for output voxels that are not covered by any input image.", us::Any(0.), true);
  parser.addArgument("super-sampling", "s", mitkCommandLineParser::StringList, "Super sampling factor", "Value used for super sampling of the result. E.g. factor 2 will lead to a doubled resolution compared to the used template. If not specified, no super sampling will be done.", us::Any(), true);
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.endGroup();
  //! [add arguments]
}

bool configureApplicationSettings(std::map<std::string, us::Any> parsedArgs)
{
  try
  {
    if (parsedArgs.size() == 0)
      return false;

    inFileName = us::any_cast<std::string>(parsedArgs["input"]);
    outFileName = us::any_cast<std::string>(parsedArgs["output"]);

    if (parsedArgs.count("template"))
    {
      refGeometryFileName = us::any_cast<std::string>(parsedArgs["template"]);
    }

    if (parsedArgs.count("registration"))
    {
      regFileName = us::any_cast<std::string>(parsedArgs["registration"]);
    }

    if (parsedArgs.count("interpolator"))
    {
      auto interpolator = us::any_cast<int>(parsedArgs["interpolator"]);
      interpolatorType = static_cast<mitk::ImageMappingInterpolator::Type>(interpolator);
    }

    if (parsedArgs.count("padding"))
    {
      paddingValue = us::any_cast<float>(parsedArgs["padding"]);
    }

    superSamplingFactors.clear();
    if (parsedArgs.count("super-sampling"))
    {
      try
      {
        auto samplingStrings = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["super-sampling"]);
        if (samplingStrings.size() != 1 && samplingStrings.size() != 3)
        {
          std::cerr << "Error. Invalid number of super sampling parameters provided. Either give one (for isometric super sampling) or 3.";
          return false;
        }

        for (const auto& samplingstr : samplingStrings)
        {
          superSamplingFactors.push_back(std::stoul(samplingstr));
        }
        if (superSamplingFactors.size() == 1)
        {
          superSamplingFactors.push_back(superSamplingFactors[0]);
          superSamplingFactors.push_back(superSamplingFactors[0]);
        }
      }
      catch (...)
      {
        std::cerr << "Error. Invalid super sampling parameter provided.";
        throw;
      }
    }
  }
  catch (...)
  {
    return false;
  }

  return true;
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  setupParser(parser);

  mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor = mitk::PreferenceListReaderOptionsFunctor({ "MITK DICOM Reader v2 (autoselect)" }, { "" });

  const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);
  if (!configureApplicationSettings(parsedArgs))
  {
    MITK_ERROR << "Command line arguments are invalid. To see the correct usage please call with -h or --help to show the help information.";
    return EXIT_FAILURE;
  };

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::cout << std::endl << "*******************************************" << std::endl;
  std::cout << "Input file:        " << inFileName << std::endl;
  std::cout << "Output file:        " << outFileName << std::endl;
  std::cout << "Registration: ";
  if (regFileName.empty())
    std::cout << "None (Identity)" << std::endl;
  else
    std::cout << regFileName << std::endl;
  std::cout << "Template: ";
  if (refGeometryFileName.empty())
    std::cout << "None (is input geometry)" << std::endl;
  else
    std::cout << refGeometryFileName << std::endl;
  std::cout << "Padding value: " << paddingValue << std::endl;
  std::cout << "Interpolation type: " << interpolatorType << std::endl;
  //check for super/sub sampling
  if (!superSamplingFactors.empty() && (superSamplingFactors.size() != 1 || superSamplingFactors[0] != 1))
  {
    std::cout << "Super sampling:";
    for (auto value : superSamplingFactors)
    {
      std::cout << " " << value;
    }
    std::cout << std::endl;
  }
  //! [do processing]
  try
  {
    std::cout << "Load input data..." << std::endl;
    inputImage = mitk::IOUtil::Load<mitk::Image>(inFileName);

    if (inputImage.IsNull())
    {
      MITK_ERROR << "Cannot load input image.";
      return EXIT_FAILURE;
    }

    std::cout << "Load registration..." << std::endl;
    if (regFileName.empty())
    {
      std::cout << "  associated registration: identity" << std::endl;
      registration = mitk::GenerateIdentityRegistration3D().GetPointer();
    }
    else
    {
      registration = mitk::IOUtil::Load<mitk::MAPRegistrationWrapper>(regFileName);
    }

    if (registration.IsNull())
    {
      MITK_ERROR << "Cannot load registration.";
      return EXIT_FAILURE;
    }

    if (refGeometryFileName != "")
    {
      std::cout << "Load reference image..." << std::endl;
      auto refImage = mitk::IOUtil::Load<mitk::Image>(refGeometryFileName, &readerFilterFunctor);
      if (refImage.IsNotNull())
      {
        refGeometry = refImage->GetGeometry();
      }
      else
      {
        MITK_ERROR << "Cannot load reference geometry image.";
        return EXIT_FAILURE;
      }
    }
    else
    {
      refGeometry = inputImage->GetGeometry();
    }

    //check for super/sub sampling
    if (!superSamplingFactors.empty() && (superSamplingFactors.size()!=1 || superSamplingFactors[0]!=1))
    {
      refGeometry = mitk::ImageMappingHelper::GenerateSuperSampledGeometry(refGeometry,
        superSamplingFactors[0],
        superSamplingFactors[1],
        superSamplingFactors[2]);
    }

    std::cout << "Map the images ..." << std::endl;

    auto output = mitk::ImageMappingHelper::map(inputImage, registration, false, paddingValue, refGeometry, true, 0, interpolatorType);

    std::cout << "Save output image: " << outFileName << std::endl;

    mitk::IOUtil::Save(output, outFileName);

    std::cout << "Processing finished." << std::endl;

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
