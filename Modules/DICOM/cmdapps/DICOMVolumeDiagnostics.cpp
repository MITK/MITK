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
#include <mitkDICOMFrameLayout.h>
#include <mitkDICOMFrameListHelper.h>

#include <mitkFileSystem.h>

#include <nlohmann/json.hpp>

namespace
{
  /** The counts a finding carries, omitting the ones it left at zero. */
  nlohmann::json Details(const mitk::DICOMFrameModelFinding& finding)
  {
    nlohmann::json details;

    if (finding.frameCount > 0) details["frame_count"] = finding.frameCount;
    if (finding.perFrameItemCount > 0) details["per_frame_item_count"] = finding.perFrameItemCount;
    if (finding.distinctRescalePairs > 0) details["distinct_rescale_pairs"] = finding.distinctRescalePairs;

    return details;
  }
}

void InitializeCommandLineParser(mitkCommandLineParser& parser)
{
  parser.setTitle("DICOM Volume Diagnostics");
  parser.setCategory("DICOM");
  parser.setDescription("Gives insights how MITK readers would convert a set of DICOM files into image volumes (e.g. number of volumes and the sorting of the files)");
  parser.setContributor("German Cancer Research Center (DKFZ)");
  parser.setArgumentPrefix("--", "-");

  parser.addArgument("only-own-series", "s", mitkCommandLineParser::Bool, "Only own series", "Analyze only files in the same directory that have the same DICOM Series UID, if a file is provided as input.", us::Any());
  parser.addArgument("check-3d", "d", mitkCommandLineParser::Bool, "Check 3D configs", "Analyze the input by using all known 3D configurations. If flag is not set all configurations (3D and 3D+t) will be used.", us::Any());
  parser.addArgument("check-3d+t", "t", mitkCommandLineParser::Bool, "Check 3D+t configs", "Analyze the input by using all known 3D+t configurations (thus dynamic image configurations). If flag is not set all configurations (3D and 3D+t) will be used.", us::Any());
  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file or path", "DICOM file or directory containing DICOM files.", us::Any(), false, false, false, mitkCommandLineParser::Input);
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
    int missingSlicesDetected = 0;

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
      bool pathIsDirectory = fs::is_directory(inputFilename);
      std::string resolvedInputFilename = inputFilename;

      if (!pathIsDirectory)
      {
        const auto listedFile = mitk::FindListedFile(inputFilename, relevantFiles);

        if (!listedFile.has_value())
        {
          mitkThrow() << "DICOM Volume Diagnostics did not find the input file among the DICOM files of its directory. Input: " << inputFilename;
        }

        resolvedInputFilename = listedFile.value();
      }

      if (!pathIsDirectory && onlyOwnSeries)
      {
        relevantFiles = mitk::FilterDICOMFilesForSameSeries(resolvedInputFilename, relevantFiles);
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
        std::stringstream config;
        reader->PrintConfiguration(config);
        readerInfo["config_details"] = config.str();

        diagnosticsResult["selected_reader"] = readerInfo;

        // The selector scans with GDCM, which cannot look into sequences.
        // BaseDICOMReaderService::DoRead analyzes with the frame model before loading, so a
        // report built on the selection scan alone would show no frame model at
        // all and answer a different question than this tool claims to.
        // AnalyzeInputFiles clears its outputs first, so re-running it is safe.
        const auto scanCache = mitk::AnalyzeWithFrameModel(*reader, relevantFiles);

        nlohmann::json outputInfos;
        nlohmann::json findings = nlohmann::json::array();
        unsigned int warningFindings = 0;
        unsigned int infoFindings = 0;

        unsigned int relevantOutputCount = 0;
        const auto nrOfOutputs = reader->GetNumberOfOutputs();
        for (std::remove_const_t<decltype(nrOfOutputs)> outputIndex = 0; outputIndex < nrOfOutputs; ++outputIndex)
        {
          const bool isRelevantOutput = pathIsDirectory
            || mitk::ContainsFile(reader->GetOutput(outputIndex).GetImageFrameList(), resolvedInputFilename);

          if (isRelevantOutput)
          {
            ++relevantOutputCount;
            nlohmann::json outputInfo;

            const auto output = reader->GetOutput(outputIndex);
            const auto frameList = output.GetImageFrameList();
            mitk::DICOMFilePathList outputFiles;
            outputFiles.resize(frameList.size());
            std::transform(frameList.begin(), frameList.end(), outputFiles.begin(), [](const mitk::DICOMImageFrameInfo::Pointer& frame) { return frame->Filename; });

            const auto distinctFiles = mitk::DistinctFilesInOrder(frameList);

            outputInfo["files"] = outputFiles;
            outputInfo["timesteps"] = output.GetNumberOfTimeSteps();
            outputInfo["frames_per_timesteps"] = output.GetNumberOfFramesPerTimeStep();
            outputInfo["frames"] = frameList.size();
            outputInfo["distinct_files"] = distinctFiles;

            bool anyFrameModel = false;

            for (const auto& distinctFile : distinctFiles)
            {
              const auto frameOfFile = std::find_if(frameList.begin(), frameList.end(), [&distinctFile](const mitk::DICOMImageFrameInfo::Pointer& frame)
                {
                  return frame->Filename == distinctFile;
                });

              const auto layout = scanCache->GetFrameLayout(*frameOfFile);
              anyFrameModel = anyFrameModel || layout.HasFrameModel();

              for (const auto& finding : mitk::CollectFrameModelFindings(layout, distinctFile))
              {
                nlohmann::json entry;
                entry["type"] = mitk::DICOMFrameModelIssueToKey(finding.issue);
                entry["severity"] = mitk::DICOMFrameModelSeverityToKey(finding.severity);
                entry["message"] = mitk::DICOMFrameModelIssueToString(finding.issue);
                entry["volume_index"] = relevantOutputCount - 1;
                entry["files"] = finding.files;
                entry["details"] = Details(finding);
                findings.push_back(entry);

                if (mitk::DICOMFrameModelSeverity::Warning == finding.severity)
                {
                  ++warningFindings;
                }
                else
                {
                  ++infoFindings;
                }
              }
            }

            outputInfo["frame_model"] = anyFrameModel;

            if (output.GetSplitReason()!=nullptr && output.GetSplitReason()->HasReasons())
            {
              outputInfo["volume_split_reason"] = mitk::IOVolumeSplitReason::ToJSON(output.GetSplitReason());

              try
              {
                if (output.GetSplitReason()->HasReason(mitk::IOVolumeSplitReason::ReasonType::MissingSlices))
                {
                  missingSlicesDetected += std::stoi(output.GetSplitReason()->GetReasonDetails(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
                }
              }
              catch (const std::exception& e)
              {
                std::cerr << "Error while checking for missing slices split reasons in volume #" << relevantOutputCount << "." << std::endl;
                std::cerr << "Error details:" << e.what() << std::endl;
              }
              catch (...)
              {
                std::cerr << "Unknown error while checking for missing slices split reasons in volume #" << relevantOutputCount << "." << std::endl;
              }
            }
            outputInfos.push_back(outputInfo);
          }
        }
        diagnosticsResult["volume_count"] = relevantOutputCount;
        diagnosticsResult["volumes"] = outputInfos;
        diagnosticsResult["findings"] = findings;
        diagnosticsResult["findings_summary"] = { { "warning", warningFindings }, { "info", infoFindings } };
      }
    }
    std::cout << "\n### DIAGNOSTICS REPORT ###\n" << std::endl;
    std::cout << std::setw(2) << diagnosticsResult << std::endl;

    if (missingSlicesDetected > 0)
    {
      std::cout << std::endl;
      std::cout << "\n!!! WARNING: MISSING SLICES !!!\n"
        "Details: Reader indicated volume splitting due to missing slices. Converted data might be invalid/incomplete.\n"
        "Estimated number of missing slices: " << missingSlicesDetected << std::endl;
    }

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
