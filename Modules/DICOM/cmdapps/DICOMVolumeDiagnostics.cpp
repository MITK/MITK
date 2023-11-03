/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommandLineParser.h>


#include <mitkIOUtil.h>
#include <mitkDICOMEnums.h>
#include <mitkDICOMFilesHelper.h>
#include <mitkDICOMFileReaderSelector.h>

#include <filesystem>

#include <nlohmann/json.hpp>

void InitializeCommandLineParser(mitkCommandLineParser& parser)
{
  parser.setTitle("DICOM Volume Diagnostics");
  parser.setCategory("DICOM");
  parser.setDescription("Gives insights how MITK readers would convert a set of DICOM files into image volumes (e.g. number of volumes and the sorting of the files)");
  parser.setContributor("German Cancer Research Center (DKFZ)");
  parser.setArgumentPrefix("--", "-");

  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("only-own-series", "s", mitkCommandLineParser::Bool, "Only own series", "Analyze only files in the same directory that have the same DICOM Series UID, if a file is provided as input.", us::Any());
  parser.addArgument("check-3d", "d", mitkCommandLineParser::Bool, "Check 3D configs", "Analyze the input by using all known 3D configurations. If flag is not set all configurations (3D and 3D+t) will be used.", us::Any());
  parser.addArgument("check-3d+t", "t", mitkCommandLineParser::Bool, "Check 3D+t configs", "Analyze the input by using all known 3D+t configurations (thus dynamic image configurations). If flag is not set all configurations (3D and 3D+t) will be used.", us::Any());
  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file or path", "Input contour(s)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file", "Output file where the diagnostics results are stored as json.", us::Any());
}

int main(int argc, char* argv[])
{
  int returnValue = EXIT_SUCCESS;

  mitkCommandLineParser parser;
  InitializeCommandLineParser(parser);

  auto args = parser.parseArguments(argc, argv);

  if (args.empty())
  {
    std::cout << parser.helpText();
    return EXIT_FAILURE;
  }

  nlohmann::json diagnosticsResult;

  try
  {
    auto inputFilename = us::any_cast<std::string>(args["input"]);
    auto outputFilename = args.count("output")==0 ? std::string() : us::any_cast<std::string>(args["output"]);
    bool onlyOwnSeries = args.count("only-own-series");
    bool check3D = args.count("check-3d");
    bool check3DPlusT = args.count("check-3d+t");

    if (!check3D && !check3DPlusT)
    { //if no check option is selected all are activated by default.
      check3D = true;
      check3DPlusT = true;
    }

    diagnosticsResult["input"] = inputFilename;
    diagnosticsResult["only-own-series"] = onlyOwnSeries;
    diagnosticsResult["check-3d"] = check3D;
    diagnosticsResult["check-3d+t"] = check3DPlusT;

    mitk::StringList relevantFiles = mitk::GetDICOMFilesInSameDirectory(inputFilename);

    if (relevantFiles.empty())
    {
      mitkThrow() << "DICOM Volume Diagnostics found no relevant files in specified location. No data is loaded. Location: " << inputFilename;
    }
    else
    {
      bool pathIsDirectory = std::filesystem::is_directory(inputFilename);

      if (!pathIsDirectory && onlyOwnSeries)
      {
        relevantFiles = mitk::FilterDICOMFilesForSameSeries(inputFilename, relevantFiles);
      }

      diagnosticsResult["analyzed_files"] = relevantFiles;

      auto selector = mitk::DICOMFileReaderSelector::New();

      if (check3D) selector->LoadBuiltIn3DConfigs();
      if (check3DPlusT) selector->LoadBuiltIn3DnTConfigs();

      nlohmann::json readerInfos;
      for (const auto& reader : selector->GetAllConfiguredReaders())
      {
        nlohmann::json readerInfo;
        readerInfo["class_name"] = reader->GetNameOfClass();
        readerInfo["configuration_label"] = reader->GetConfigurationLabel();
        readerInfo["configuration_description"] = reader->GetConfigurationDescription();
        readerInfos.push_back(readerInfo);
      }
      diagnosticsResult["checked_readers"] = readerInfos;

      selector->SetInputFiles(relevantFiles);

      auto reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();

      if (reader.IsNull())
      {
        mitkThrow() << "DICOM Volume Diagnostics service found no suitable reader configuration for relevant files.";
      }
      else
      {
        nlohmann::json readerInfo;
        readerInfo["class_name"] = reader->GetNameOfClass();
        readerInfo["configuration_label"] = reader->GetConfigurationLabel();
        readerInfo["configuration_description"] = reader->GetConfigurationDescription();
        readerInfo["configuration_description"] = reader->GetConfigurationDescription();
        std::stringstream config;
        reader->PrintConfiguration(config);
        readerInfo["config_details"] = config.str();

        diagnosticsResult["selected_reader"] = readerInfo;

        nlohmann::json outputInfos;

        unsigned int relevantOutputCount = 0;
        const auto nrOfOutputs = reader->GetNumberOfOutputs();
        for (std::remove_const_t<decltype(nrOfOutputs)> outputIndex = 0; outputIndex < nrOfOutputs; ++outputIndex)
        {
          bool isRelevantOutput = true;
          if (!pathIsDirectory)
          {
            const auto frameList = reader->GetOutput(outputIndex).GetImageFrameList();
            auto finding = std::find_if(frameList.begin(), frameList.end(), [&](const mitk::DICOMImageFrameInfo::Pointer& frame)
              {
                std::filesystem::path framePath(frame->Filename);
                std::filesystem::path inputPath(inputFilename);
                return framePath == inputPath;
              });
            isRelevantOutput = finding != frameList.end();
          }

          if (isRelevantOutput)
          {
            ++relevantOutputCount;
            nlohmann::json outputInfo;

            const auto output = reader->GetOutput(outputIndex);
            const auto frameList = output.GetImageFrameList();
            mitk::DICOMFilePathList outputFiles;
            outputFiles.resize(frameList.size());
            std::transform(frameList.begin(), frameList.end(), outputFiles.begin(), [](const mitk::DICOMImageFrameInfo::Pointer& frame) { return frame->Filename; });

            outputInfo["files"] = outputFiles;
            outputInfo["timesteps"] = output.GetNumberOfTimeSteps();
            outputInfo["frames_per_timesteps"] = output.GetNumberOfFramesPerTimeStep();
            outputInfos.push_back(outputInfo);
          }
        }
        diagnosticsResult["volume_count"] = relevantOutputCount;
        diagnosticsResult["volumes"] = outputInfos;
      }
    }
    std::cout << "\n### DIAGNOSTICS REPORT ###\n" << std::endl;
    std::cout << std::setw(2) << diagnosticsResult << std::endl;

    if (!outputFilename.empty())
    {
      std::ofstream fileout(outputFilename);
      fileout << diagnosticsResult;
      fileout.close();
    }

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
    MITK_ERROR << "An unknown error occurred!";
    return EXIT_FAILURE;
  }

  return returnValue;
}
