/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkProperties.h"

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"

#include <itksys/SystemTools.hxx>

#include "mitkPreferenceListReaderOptionsFunctor.h"


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("File Converter");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file:", "Input File",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("reader", "r", mitkCommandLineParser::String, "Reader Name", "Reader Name", us::Any());
  parser.addArgument("list-readers", "lr", mitkCommandLineParser::Bool, "Reader Name", "Reader Name", us::Any());


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

  auto nodes = mitk::IOUtil::Load(inputFilename, &functor);

  unsigned count = 0;
  for (auto node : nodes)
  {
    std::string writeName = path + "/" + filename + extension;
    if (count > 0)
    {
      writeName = path + "/" + filename + "_" + std::to_string(count) + extension;
    }
    mitk::IOUtil::Save(node, writeName);
    ++count;
  }

  return EXIT_SUCCESS;
}
