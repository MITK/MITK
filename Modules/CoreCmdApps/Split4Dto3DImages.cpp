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
#include <mitkImageTimeSelector.h>
#include <mitkTemporoSpatialStringProperty.h>

std::string inFileName;
std::string outFileNamePattern;

void setupParser(mitkCommandLineParser& parser)
{
    // set general information
    parser.setCategory("Dynamic Data Analysis Tools");
    parser.setTitle("Split 4D to 3D Image");
    parser.setDescription("CLI app that allows to split an image into the images per time point. Time resolved properties will be also split.");
    parser.setContributor("DKFZ MIC");
    //! [create parser]

    //! [add arguments]
    // how should arguments be prefixed
    parser.setArgumentPrefix("--", "-");
    // add each argument, unless specified otherwise each argument is optional
    // see mitkCommandLineParser::addArgument for more information
    parser.beginGroup("Required I/O parameters");
    parser.addArgument(
        "input", "i", mitkCommandLineParser::File, "Input file", "Path to the input image that should be splitted. If the image has only one time point it will be stored as output untouched.", us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("output",
        "o",
        mitkCommandLineParser::File,
        "Output file(s) path",
        "Path to the splitted images. If the input has multiple time points the path will be used as pattern and a suffix \"_[time step]\" will be added before the extension.",
        us::Any(),
        false, false, false, mitkCommandLineParser::Output);
    parser.endGroup();

    parser.beginGroup("Optional parameters");
    parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
    parser.endGroup();
    //! [add arguments]
}

bool configureApplicationSettings(std::map<std::string, us::Any> parsedArgs)
{
    if (parsedArgs.size() == 0)
        return false;

    inFileName = us::any_cast<std::string>(parsedArgs["input"]);
    outFileNamePattern = us::any_cast<std::string>(parsedArgs["output"]);

    return true;
}

void transferMetaProperties(const mitk::Image* sourceImage, mitk::Image* destinationImage, mitk::TimeStepType ts)
{
  auto props = sourceImage->GetPropertyList()->GetMap();

  for (const auto& [name, prop] : *props)
  {
    auto tsProperty = dynamic_cast<const mitk::TemporoSpatialStringProperty*>(prop.GetPointer());
    if (nullptr == tsProperty)
    {
      destinationImage->SetProperty(name, prop->Clone());
    }
    else
    {
      if (tsProperty->HasValueByTimeStep(ts))
      {
        auto extractedProp = ExtractTimeStepFromTemporoSpatialStringProperty(tsProperty, ts);
        destinationImage->SetProperty(name, extractedProp);
      }
    }
  }
}

void saveResultImage(const mitk::Image* image, const std::string& fileName, mitk::TimeStepType ts)
{
  std::cout << "time step: "<< ts << " -> " <<fileName << std::endl;
  mitk::IOUtil::Save(image, fileName);
}

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    setupParser(parser);

    mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor = mitk::PreferenceListReaderOptionsFunctor({ "MITK DICOM Reader v2 (autoselect)" }, { "" });

    const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);
    if (!configureApplicationSettings(parsedArgs))
    {
        return EXIT_FAILURE;
    };

    // Show a help message
    if (parsedArgs.count("help") || parsedArgs.count("h"))
    {
        std::cout << parser.helpText();
        return EXIT_SUCCESS;
    }

    //! [do processing]
    try
    {
      std::cout << "Load image:" <<  inFileName << std::endl;
      auto image = mitk::IOUtil::Load<mitk::Image>(inFileName, &readerFilterFunctor);

      std::cout << "Split the image ..." << std::endl;

      if (image->GetTimeGeometry()->CountTimeSteps() == 1)
      {
        std::cout << "Input contains only one time step; will be passed through" << std::endl;
        saveResultImage(image, outFileNamePattern, 0);
      }
      else
      {
        std::string extension = itksys::SystemTools::GetFilenameExtension(outFileNamePattern);
        std::string filename = itksys::SystemTools::GetFilenameWithoutExtension(outFileNamePattern);
        std::string path = itksys::SystemTools::GetFilenamePath(outFileNamePattern);
        if (!path.empty())
        {
          path = path + mitk::IOUtil::GetDirectorySeparator();
        }

        std::cout << "Process time steps:" << std::endl;
        for (mitk::TimeStepType ts = 0; ts < image->GetTimeGeometry()->CountTimeSteps(); ++ts)
        {
          auto splitImage = mitk::SelectImageByTimeStep(image, ts)->Clone();
          transferMetaProperties(image, splitImage, ts);

          std::string writeName = path + filename + "_" + std::to_string(ts) + extension;

          saveResultImage(splitImage, writeName, ts);
        }
        std::cout << std::endl;
      }

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
