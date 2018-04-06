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

#include <mitkCommon.h>
#include <chrono>
#include <mitkIOUtil.h>
#include <mitkCommandLineParser.h>
#include <mitkUIDGenerator.h>
#include <mitkException.h>

#include <itksys/SystemTools.hxx>

struct InputParameters
{
  std::string saveFolderPath;
  std::string identifyer;
  std::string exePath;
  std::string probePath;
  bool verbose;
};

InputParameters parseInput(int argc, char* argv[])
{
  MITK_INFO << "Paring arguments...";
  mitkCommandLineParser parser;
  // set general information
  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Photoacoustics Beamforming Tool");
  parser.setDescription("Reads a nrrd file as an input and applies a beamforming method as set with the parameters.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  // how should arguments be prefixed
  parser.setArgumentPrefix("--", "-");
  // add each argument, unless specified otherwise each argument is optional
  // see mitkCommandLineParser::addArgument for more information
  parser.beginGroup("Required parameters");
  parser.addArgument(
    "savePath", "s", mitkCommandLineParser::InputDirectory,
    "Input save folder (directory)", "input save folder",
    us::Any(), false);
  parser.endGroup();
  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "probe", "p", mitkCommandLineParser::OutputFile,
    "xml probe file (file)", "file to the definition of the used probe (*.xml)",
    us::Any());
  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool,
    "Verbose Output", "Whether to produce verbose, or rather debug output");
  parser.addArgument(
    "identifyer", "i", mitkCommandLineParser::String,
    "Generator identifyer (string)", "A unique identifyer for the calculation instance");
  parser.endGroup();

  InputParameters input;

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    exit(-1);

  if (parsedArgs.count("verbose"))
  {
    MITK_INFO << "verbose";
    input.verbose = us::any_cast<bool>(parsedArgs["verbose"]);
  }
  else
  {
    input.verbose = false;
  }

  if (parsedArgs.count("savePath"))
  {
    MITK_INFO << "savePath";
    input.saveFolderPath = us::any_cast<std::string>(parsedArgs["savePath"]);
  }

  if (parsedArgs.count("mitkMcxyz"))
  {
    MITK_INFO << "mitkMcxyz";
    input.exePath = us::any_cast<std::string>(parsedArgs["mitkMcxyz"]);
  }

  if (parsedArgs.count("probe"))
  {
    MITK_INFO << "probe";
    input.probePath = us::any_cast<std::string>(parsedArgs["probe"]);
  }

  if (parsedArgs.count("identifyer"))
  {
    MITK_INFO << "identifyer";
    input.identifyer = us::any_cast<std::string>(parsedArgs["identifyer"]);
  }
  else
  {
    MITK_INFO << "generating identifyer";
    auto uid = mitk::UIDGenerator("", 8);
    input.identifyer = uid.GetUID();
  }
  MITK_INFO << "Paring arguments...[Done]";
  return input;
}

int main(int argc, char * argv[])
{
  auto input = parseInput(argc, argv);
  MITK_INFO << "We did something!";
}
