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

mitkCommandLineParser::StringContainerType inFilenames;
mitkCommandLineParser::StringContainerType regFilenames;
std::string outFileName;
std::string refGeometryFileName;

std::vector<mitk::Image::ConstPointer> images;
std::vector<mitk::MAPRegistrationWrapper::ConstPointer> registrations;
mitk::BaseGeometry::Pointer refGeometry;
mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear;
double paddingValue = 0;
itk::StitchStrategy stitchStratgy = itk::StitchStrategy::Mean;

void setupParser(mitkCommandLineParser& parser)
{
    // set general information about your MiniApp
    parser.setCategory("Mapping Tools");
    parser.setTitle("Stitch 3D Images");
    parser.setDescription("MiniApp that allows to map and stitch 3D images into a given output geometry.");
    parser.setContributor("DKFZ MIC");
    //! [create parser]

    //! [add arguments]
    // how should arguments be prefixed
    parser.setArgumentPrefix("--", "-");
    // add each argument, unless specified otherwise each argument is optional
    // see mitkCommandLineParser::addArgument for more information
    parser.beginGroup("Required I/O parameters");
    parser.addArgument(
        "inputs", "i", mitkCommandLineParser::StringList, "Input files", "Pathes to the input images that should be mapped and stitched", us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("output",
        "o",
        mitkCommandLineParser::File,
        "Output file path",
        "Path to the fused 3D+t image.",
        us::Any(),
        false, false, false, mitkCommandLineParser::Output);
    parser.endGroup();

    parser.beginGroup("Optional parameters");
    parser.addArgument("template",
      "t",
      mitkCommandLineParser::File,
      "Output template image.",
      "File path to an image that serves as template for the output geometry.",
      us::Any(),
      false, false, false, mitkCommandLineParser::Input);
    parser.addArgument(
      "registrations", "r", mitkCommandLineParser::StringList, "Registration files", "Pathes to the registrations that should be used to map the input images. If this parameter is not set, identity transforms are assumed. If this parameter is set, it must have the same number of entries then the parameter inputs. If you want to use and identity transform for a specific input, specify an empty string. The application assumes that inputs and registrations have the same order, so the n-th input should use thr n-th registration.", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument("interpolator", "n", mitkCommandLineParser::Int, "Interpolator type", "Interpolator used for mapping the images. Default: 2; allowed values: 1: Nearest Neighbour, 2: Linear, 3: BSpline 3, 4: WSinc Hamming, 5: WSinc Welch", us::Any(2), true);
    parser.addArgument("strategy", "s", mitkCommandLineParser::Int, "Stitch strategy", "Strategy used for stitching the images. 0: Mean -> computes the mean value of all input images that cover an output pixel (default strategy). 1: BorderDistance -> Uses the input pixel that has the largest minimal distance to its image borders", us::Any(2), true);
    parser.addArgument("padding", "p", mitkCommandLineParser::Float, "Padding value", "Value used for output voxels that are not covered by any input image.", us::Any(0.), true);
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

    inFilenames = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["inputs"]);
    outFileName = us::any_cast<std::string>(parsedArgs["output"]);

    if (parsedArgs.count("template"))
    {
      refGeometryFileName = us::any_cast<std::string>(parsedArgs["template"]);
    }

    if (parsedArgs.count("registrations"))
    {
      regFilenames = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["registrations"]);
    }
    else
    {
      regFilenames.resize(inFilenames.size());
      std::fill(regFilenames.begin(), regFilenames.end(), "");
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

    if (parsedArgs.count("strategy"))
    {
      auto temp = us::any_cast<int>(parsedArgs["strategy"]);
      stitchStratgy = static_cast<itk::StitchStrategy>(temp);
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

    if(regFilenames.size() != inFilenames.size())
    {
      MITK_ERROR << "Cannot stitch inputs. The number of specified registrations does not match the number of inputs.";
      return EXIT_FAILURE;
    }

    //! [do processing]
    try
    {
      std::cout << "Load images:" << std::endl;

      unsigned int index = 0;
      for (auto path : inFilenames)
      {
        std::cout << "#"<<index<<" " << path << std::endl;
        auto image = mitk::IOUtil::Load<mitk::Image>(path, &readerFilterFunctor);
        images.push_back(image.GetPointer());
        if (regFilenames[index].empty())
        {
          std::cout << "  associated registration: identity" << std::endl;
          registrations.push_back(mitk::GenerateIdentityRegistration3D().GetPointer());
        }
        else
        {
          std::cout << "  associated registration: " << regFilenames[index] << std::endl;
          auto reg = mitk::IOUtil::Load<mitk::MAPRegistrationWrapper>(regFilenames[index]);
          registrations.push_back(reg.GetPointer());
        }
        ++index;
      }
      std::cout << "Reference image: " << refGeometryFileName << std::endl << std::endl;
      auto refImage = mitk::IOUtil::Load<mitk::Image>(refGeometryFileName, &readerFilterFunctor);
      if (refImage.IsNotNull())
      {
        refGeometry = refImage->GetGeometry();
      }
      std::cout << "Padding value: " << paddingValue << std::endl;
      std::cout << "Stitch strategy: ";
      if (itk::StitchStrategy::Mean == stitchStratgy)
      {
        std::cout << "Mean " << std::endl;
      }
      else
      {
        std::cout << "BorderDistance" << std::endl;
      }

      std::cout << "Stitch the images ..." << std::endl;

      auto output = mitk::StitchImages(images, registrations, refGeometry,paddingValue,stitchStratgy,interpolatorType);

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
