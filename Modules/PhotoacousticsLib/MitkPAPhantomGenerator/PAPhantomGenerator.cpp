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
#include <mitkPATissueGeneratorParameters.h>
#include <mitkPATissueGenerator.h>
#include <mitkPAPhantomTissueGenerator.h>
#include <mitkIOUtil.h>
#include <mitkCommandLineParser.h>
#include <mitkUIDGenerator.h>
#include <mitkException.h>

#include <itksys/SystemTools.hxx>

using namespace mitk::pa;

TissueGeneratorParameters::Pointer CreatePhantom_04_04_18_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(12);
  returnParameters->SetMinBackgroundAbsorption(0.1);
  returnParameters->SetMaxBackgroundAbsorption(0.1);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateNewDirectionVectorInStraightLine);
  returnParameters->SetDoPartialVolume(true);
  returnParameters->SetMinNumberOfVessels(1);
  returnParameters->SetMaxNumberOfVessels(8);
  returnParameters->SetMinVesselAbsorption(1);
  returnParameters->SetMaxVesselAbsorption(10);
  returnParameters->SetMinVesselAnisotropy(0.9);
  returnParameters->SetMaxVesselAnisotropy(0.9);
  returnParameters->SetMinVesselBending(0.1);
  returnParameters->SetMaxVesselBending(0.3);
  returnParameters->SetMinVesselRadiusInMillimeters(0.25);
  returnParameters->SetMaxVesselRadiusInMillimeters(4);
  returnParameters->SetMinVesselScattering(15);
  returnParameters->SetMaxVesselScattering(15);
  returnParameters->SetMinVesselZOrigin(1.6);
  returnParameters->SetMaxVesselZOrigin(4);
  returnParameters->SetVesselBifurcationFrequency(5000);
  returnParameters->SetRandomizePhysicalProperties(false);
  returnParameters->SetSkinThicknessInMillimeters(0);
  returnParameters->SetUseRngSeed(false);
  returnParameters->SetVoxelSpacingInCentimeters(0.03);
  returnParameters->SetXDim(140);
  returnParameters->SetYDim(100);
  returnParameters->SetZDim(180);
  //returnParameters->SetVoxelSpacingInCentimeters(0.015);
  //returnParameters->SetXDim(280);
  //returnParameters->SetYDim(200);
  //returnParameters->SetZDim(360);
  returnParameters->SetForceVesselsMoveAlongYDirection(true);
  //returnParameters->SetVoxelSpacingInCentimeters(0.0075);
  //returnParameters->SetXDim(560);
  //returnParameters->SetYDim(400);
  //returnParameters->SetZDim(720);
  return returnParameters;
}

struct InputParameters
{
  std::string saveFolderPath;
  std::string identifyer;
  std::string exePath;
  std::string probePath;
  bool empty;
  bool verbose;
};

InputParameters parseInput(int argc, char* argv[])
{
  MITK_INFO << "Parsing arguments...";
  mitkCommandLineParser parser;

  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk Tissue Batch Generator");
  parser.setDescription("Creates in silico tissue in batch processing and automatically calculates fluence values for the central slice of the volume.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("Required parameters");
  parser.addArgument(
    "savePath", "s", mitkCommandLineParser::Directory,
    "Input save folder (directory)", "input save folder",
    us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "mitkMcxyz", "m", mitkCommandLineParser::File,
    "MitkMcxyz binary (file)", "path to the MitkMcxyz binary",
    us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "probe", "p", mitkCommandLineParser::File,
    "xml probe file (file)", "file to the definition of the used probe (*.xml)",
    us::Any(), true, false, false, mitkCommandLineParser::Output);
  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool,
    "Verbose Output", "Whether to produce verbose, or rather debug output");
  parser.addArgument(
    "identifyer", "i", mitkCommandLineParser::String,
    "Generator identifyer (string)", "A unique identifyer for the calculation instance");
  parser.addArgument(
    "empty-volume", "e", mitkCommandLineParser::Bool,
    "omit vessel structures (boolean flag)", "Whether to create an empty volume with no structures inside.");
  parser.endGroup();

  InputParameters input;

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    exit(-1);

  if (parsedArgs.count("empty-volume"))
  {
    input.empty = us::any_cast<bool>(parsedArgs["empty-volume"]);
  }
  else
  {
    input.empty = false;
  }

  if (parsedArgs.count("verbose"))
  {
    input.verbose = us::any_cast<bool>(parsedArgs["verbose"]);
  }
  else
  {
    input.verbose = false;
  }

  if (parsedArgs.count("savePath"))
  {
    input.saveFolderPath = us::any_cast<std::string>(parsedArgs["savePath"]);
  }

  if (parsedArgs.count("mitkMcxyz"))
  {
    input.exePath = us::any_cast<std::string>(parsedArgs["mitkMcxyz"]);
  }

  if (parsedArgs.count("probe"))
  {
    input.probePath = us::any_cast<std::string>(parsedArgs["probe"]);
  }

  if (parsedArgs.count("identifyer"))
  {
    input.identifyer = us::any_cast<std::string>(parsedArgs["identifyer"]);
  }
  else
  {
    auto uid = mitk::UIDGenerator("", 8);
    input.identifyer = uid.GetUID();
  }
  MITK_INFO << "Parsing arguments...[Done]";
  return input;
}

int main(int argc, char * argv[])
{
  auto input = parseInput(argc, argv);
  auto parameters = CreatePhantom_04_04_18_Parameters();
  if (input.empty)
  {
    parameters->SetMaxNumberOfVessels(0);
    parameters->SetMinNumberOfVessels(0);
  }
  MITK_INFO(input.verbose) << "Generating tissue..";
  auto resultTissue = InSilicoTissueGenerator::GenerateInSilicoData(parameters);
  MITK_INFO(input.verbose) << "Generating tissue..[Done]";

  auto inputfolder = std::string(input.saveFolderPath + "input/");
  auto outputfolder = std::string(input.saveFolderPath + "output/");
  if (!itksys::SystemTools::FileIsDirectory(inputfolder))
  {
    itksys::SystemTools::MakeDirectory(inputfolder);
  }
  if (!itksys::SystemTools::FileIsDirectory(outputfolder))
  {
    itksys::SystemTools::MakeDirectory(outputfolder);
  }

  std::string savePath = input.saveFolderPath + "input/Phantom_" + input.identifyer +
    ".nrrd";
  mitk::IOUtil::Save(resultTissue->ConvertToMitkImage(), savePath);
  std::string outputPath = input.saveFolderPath + "output/Phantom_" + input.identifyer +
    "/";

  resultTissue = nullptr;

  if (!itksys::SystemTools::FileIsDirectory(outputPath))
  {
    itksys::SystemTools::MakeDirectory(outputPath);
  }

  outputPath = outputPath + "Fluence_Phantom_" + input.identifyer;

  MITK_INFO(input.verbose) << "Simulating fluence..";

  int result = -4;

  std::string cmdString = std::string(input.exePath + " -i " + savePath + " -o " +
    (outputPath + ".nrrd") +
    " -yo " + "0" + " -p " + input.probePath +
    " -n 10000000");

  MITK_INFO << "Executing: " << cmdString;

  result = std::system(cmdString.c_str());

  MITK_INFO << result;
  MITK_INFO(input.verbose) << "Simulating fluence..[Done]";
}
