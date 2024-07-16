/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkProperties.h"

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"

#include <itksys/SystemTools.hxx>

#include "mitkPreferenceListReaderOptionsFunctor.h"
#include "mitkIOMetaInformationPropertyConstants.h"
#include "mitkIOVolumeSplitReason.h"

#include "mitkPropertyKeyPath.h"

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("File Converter");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file:", "Input path that should be loaded.",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output path where the result should be stored. If the input generates multiple outputs the index will be added for all but the first output (before the extension; starting with 0).", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("reader", "r", mitkCommandLineParser::String, "Reader Name", "Enforce a certain reader to be used for loading the input.", us::Any());
  parser.addArgument("list-readers", "lr", mitkCommandLineParser::Bool, "List reader names", "Print names of all available readers.", us::Any());


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputFilename = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outputFilename = us::any_cast<std::string>(parsedArgs["output"]);

  mitk::PreferenceListReaderOptionsFunctor::ListType preference = {};
  if (parsedArgs.count("reader"))
  {
    preference.push_back(us::any_cast<std::string>(parsedArgs["reader"]));
  }

  if (parsedArgs.count("list-readers"))
  {
    mitk::IOUtil::LoadInfo loadInfo(inputFilename);
    auto readers = loadInfo.m_ReaderSelector.Get();

    std::string errMsg;
    if (readers.empty())
    {
      if (!itksys::SystemTools::FileExists(loadInfo.m_Path.c_str()))
      {
        errMsg += "File '" + loadInfo.m_Path + "' does not exist\n";
      }
      else
      {
        errMsg += "No reader available for '" + loadInfo.m_Path + "'\n";
      }
      MITK_ERROR << errMsg;
      return 0;
    }

    std::cout << "Available Readers: "<<std::endl << "------------------------" << std::endl;
    for (auto reader : loadInfo.m_ReaderSelector.Get())
    {
      std::cout  << " : " << reader.GetDescription() << std::endl;
    }
    return 0;
  }

  mitk::PreferenceListReaderOptionsFunctor::ListType emptyList = {};
  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, emptyList);

  std::string extension = itksys::SystemTools::GetFilenameExtension(outputFilename);
  std::string filename = itksys::SystemTools::GetFilenameWithoutExtension(outputFilename);
  std::string path = itksys::SystemTools::GetFilenamePath(outputFilename);
  if (!path.empty())
  {
    path = path + mitk::IOUtil::GetDirectorySeparator();
  }

  auto nodes = mitk::IOUtil::Load(inputFilename, &functor);

  unsigned count = 0;
  int missingSlicesDetected = 0;

  for (auto node : nodes)
  {
    std::string writeName = path + filename + extension;
    if (count > 0)
    {
      writeName = path + filename + "_" + std::to_string(count) + extension;
    }
    mitk::IOUtil::Save(node, writeName);
    ++count;

    try
    {
      auto splitReasonProperty = node->GetProperty(mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::VOLUME_SPLIT_REASON()).c_str());
      if (splitReasonProperty.IsNotNull())
      {
        auto reasonStr = splitReasonProperty->GetValueAsString();
        auto reason = mitk::IOVolumeSplitReason::FromJSON(reasonStr);
        if (reason.IsNotNull() && reason->HasReason(mitk::IOVolumeSplitReason::ReasonType::MissingSlices))
        {
          missingSlicesDetected += std::stoi(reason->GetReasonDetails(mitk::IOVolumeSplitReason::ReasonType::MissingSlices));
        }
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "Error while checking for existing split reasons in volume #" << count << "." << std::endl;
      std::cerr << "Error details:" << e.what() << std::endl;
    }
    catch (...)
    {
      std::cerr << "Unknown error while checking for existing split reasons in volume #" << count << "." << std::endl;
    }
  }

  if (missingSlicesDetected > 0)
  {
    std::cout << std::endl;
    std::cout << "\n!!! WARNING: MISSING SLICES !!!\n"
                 "Details: Reader indicated volume splitting due to missing slices. Converted data might be invalid/incomplete.\n"
                 "Estimated number of missing slices: " << missingSlicesDetected << std::endl;
  }

  return EXIT_SUCCESS;
}
