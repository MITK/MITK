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
#include <mitkTemporalJoinImagesFilter.h>

mitkCommandLineParser::StringContainerType inFilenames;
std::string outFileName;

std::vector<mitk::Image::Pointer> images;
std::vector<mitk::TimePointType> timebounds;

void setupParser(mitkCommandLineParser& parser)
{
    // set general information about your MiniApp
    parser.setCategory("Dynamic Data Analysis Tools");
    parser.setTitle("Fuse 3D to 4D Image");
    parser.setDescription("MiniApp that allows to fuse several 3D images (with same geometry) into a 3D+t (4D) image that can be processed as dynamic data.");
    parser.setContributor("DKFZ MIC");
    //! [create parser]

    //! [add arguments]
    // how should arguments be prefixed
    parser.setArgumentPrefix("--", "-");
    // add each argument, unless specified otherwise each argument is optional
    // see mitkCommandLineParser::addArgument for more information
    parser.beginGroup("Required I/O parameters");
    parser.addArgument(
        "inputs", "i", mitkCommandLineParser::StringList, "Input files", "Pathes to the input images that should be fused", us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("output",
        "o",
        mitkCommandLineParser::File,
        "Output file path",
        "Path to the fused 3D+t image.",
        us::Any(),
        false, false, false, mitkCommandLineParser::Output);
    parser.endGroup();

    parser.beginGroup("Optional parameters");
    parser.addArgument(
        "time", "t", mitkCommandLineParser::StringList, "Time bounds", "Defines the time geometry of the resulting dynamic image in [ms]. The first number is the start time point of the first time step. All other numbers are the max bound of a time step. So the structure is [minBound0 maxBound1 [maxBound2 [... maxBoundN]]]; e.g. \"2 3.5 10\" encodes a time geometry with two time steps and that starts at 2 ms and the second time step starts at 3.5 ms and ends at 10 ms. If not set e propertional time geometry with 1 ms duration will be generated!", us::Any(), true, false, false, mitkCommandLineParser::Input);
    parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
    parser.endGroup();
    //! [add arguments]
}

bool configureApplicationSettings(std::map<std::string, us::Any> parsedArgs)
{
    if (parsedArgs.size() == 0)
        return false;

    inFilenames = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["inputs"]);
    outFileName = us::any_cast<std::string>(parsedArgs["output"]);

    if (parsedArgs.count("time"))
    {
        auto timeBoundsStr = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["time"]);
        for (const auto& timeBoundStr : timeBoundsStr)
        {
          std::istringstream stream;
          stream.imbue(std::locale("C"));
          stream.str(timeBoundStr);
          mitk::TimePointType time = 0 ;
          stream >> time;
          timebounds.emplace_back(time);
        }
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
        return EXIT_FAILURE;
    };

    // Show a help message
    if (parsedArgs.count("help") || parsedArgs.count("h"))
    {
        std::cout << parser.helpText();
        return EXIT_SUCCESS;
    }

    if (timebounds.empty())
    {
      timebounds.resize(inFilenames.size()+1);
      std::iota(timebounds.begin(), timebounds.end(), 0.0);
    }
    else if (inFilenames.size() + 1 != timebounds.size())
    {
      std::cerr << "Cannot fuse images. Explicitly specified time bounds do not match. Use --help for more information on how to specify time bounds correctly.";
      return EXIT_FAILURE;
    };

    //! [do processing]
    try
    {
      std::cout << "Load images:" << std::endl;

      auto filter = mitk::TemporalJoinImagesFilter::New();

      unsigned int step = 0;
      for (auto path : inFilenames)
      {
        std::cout << "Time step #"<<step<<" @ "<<timebounds[step]<< " ms: " << path << std::endl;
        auto image = mitk::IOUtil::Load<mitk::Image>(path, &readerFilterFunctor);
        images.push_back(image);
        filter->SetInput(step, image);
        ++step;
      }

      filter->SetFirstMinTimeBound(timebounds[0]);
      filter->SetMaxTimeBounds({ timebounds.begin() + 1, timebounds.end() });

      std::cout << "Fuse the images ..." << std::endl;

      filter->Update();
      auto output = filter->GetOutput();

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
