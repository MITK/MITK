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
#include <mitkIOUtil.h>
#include <mitkCommandLineParser.h>
#include <mitkUIDGenerator.h>
#include <mitkException.h>

#include <itksys/SystemTools.hxx>

using namespace mitk::pa;

TissueGeneratorParameters::Pointer CreateMultiHB_13_02_18_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(1.8);
  returnParameters->SetMinBackgroundAbsorption(0.001);
  returnParameters->SetMaxBackgroundAbsorption(0.2);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateNewRandomlyDivergingDirectionVector);
  returnParameters->SetDoPartialVolume(true);
  returnParameters->SetMinNumberOfVessels(1);
  returnParameters->SetMaxNumberOfVessels(7);
  returnParameters->SetMinVesselAbsorption(1);
  returnParameters->SetMaxVesselAbsorption(12);
  returnParameters->SetMinVesselAnisotropy(0.9);
  returnParameters->SetMaxVesselAnisotropy(0.9);
  returnParameters->SetMinVesselBending(0);
  returnParameters->SetMaxVesselBending(0.2);
  returnParameters->SetMinVesselRadiusInMillimeters(0.5);
  returnParameters->SetMaxVesselRadiusInMillimeters(6);
  returnParameters->SetMinVesselScattering(15);
  returnParameters->SetMaxVesselScattering(15);
  returnParameters->SetMinVesselZOrigin(1);
  returnParameters->SetMaxVesselZOrigin(3);
  returnParameters->SetVesselBifurcationFrequency(5000);
  returnParameters->SetRandomizePhysicalProperties(false);
  returnParameters->SetSkinThicknessInMillimeters(0);
  returnParameters->SetUseRngSeed(false);
  returnParameters->SetVoxelSpacingInCentimeters(0.06);
  returnParameters->SetXDim(70);
  returnParameters->SetYDim(100);
  returnParameters->SetZDim(100);
  returnParameters->SetMCflag(4);
  return returnParameters;
}

TissueGeneratorParameters::Pointer CreateBaselineHB_13_02_18_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(1.8);
  returnParameters->SetMinBackgroundAbsorption(0.001);
  returnParameters->SetMaxBackgroundAbsorption(0.2);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateNewRandomlyDivergingDirectionVector);
  returnParameters->SetDoPartialVolume(true);
  returnParameters->SetMinNumberOfVessels(1);
  returnParameters->SetMaxNumberOfVessels(1);
  returnParameters->SetMinVesselAbsorption(4.73);
  returnParameters->SetMaxVesselAbsorption(4.73);
  returnParameters->SetMinVesselAnisotropy(0.9);
  returnParameters->SetMaxVesselAnisotropy(0.9);
  returnParameters->SetMinVesselBending(0);
  returnParameters->SetMaxVesselBending(0.2);
  returnParameters->SetMinVesselRadiusInMillimeters(3);
  returnParameters->SetMaxVesselRadiusInMillimeters(3);
  returnParameters->SetMinVesselScattering(15);
  returnParameters->SetMaxVesselScattering(15);
  returnParameters->SetMinVesselZOrigin(1);
  returnParameters->SetMaxVesselZOrigin(3);
  returnParameters->SetVesselBifurcationFrequency(5000);
  returnParameters->SetRandomizePhysicalProperties(false);
  returnParameters->SetSkinThicknessInMillimeters(0);
  returnParameters->SetUseRngSeed(false);
  returnParameters->SetVoxelSpacingInCentimeters(0.06);
  returnParameters->SetXDim(70);
  returnParameters->SetYDim(100);
  returnParameters->SetZDim(100);
  returnParameters->SetMCflag(4);
  return returnParameters;
}

TissueGeneratorParameters::Pointer CreateSingleVesselHeterogeneousBackground_08_02_18_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(1.8);
  returnParameters->SetMinBackgroundAbsorption(0.001);
  returnParameters->SetMaxBackgroundAbsorption(0.2);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateNewRandomlyDivergingDirectionVector);
  returnParameters->SetDoPartialVolume(true);
  returnParameters->SetMinNumberOfVessels(1);
  returnParameters->SetMaxNumberOfVessels(1);
  returnParameters->SetMinVesselAbsorption(1);
  returnParameters->SetMaxVesselAbsorption(12);
  returnParameters->SetMinVesselAnisotropy(0.9);
  returnParameters->SetMaxVesselAnisotropy(0.9);
  returnParameters->SetMinVesselBending(0);
  returnParameters->SetMaxVesselBending(0.2);
  returnParameters->SetMinVesselRadiusInMillimeters(0.5);
  returnParameters->SetMaxVesselRadiusInMillimeters(6);
  returnParameters->SetMinVesselScattering(15);
  returnParameters->SetMaxVesselScattering(15);
  returnParameters->SetMinVesselZOrigin(1);
  returnParameters->SetMaxVesselZOrigin(3);
  returnParameters->SetVesselBifurcationFrequency(5000);
  returnParameters->SetRandomizePhysicalProperties(false);
  returnParameters->SetSkinThicknessInMillimeters(0);
  returnParameters->SetUseRngSeed(false);
  returnParameters->SetVoxelSpacingInCentimeters(0.06);
  returnParameters->SetXDim(70);
  returnParameters->SetYDim(100);
  returnParameters->SetZDim(100);
  returnParameters->SetMCflag(4);
  return returnParameters;
}

TissueGeneratorParameters::Pointer CreateMultivessel_19_12_17_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(12);
  returnParameters->SetMinBackgroundAbsorption(0.1);
  returnParameters->SetMaxBackgroundAbsorption(0.1);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateNewRandomlyDivergingDirectionVector);
  returnParameters->SetDoPartialVolume(true);
  returnParameters->SetMinNumberOfVessels(1);
  returnParameters->SetMaxNumberOfVessels(7);
  returnParameters->SetMinVesselAbsorption(2);
  returnParameters->SetMaxVesselAbsorption(8);
  returnParameters->SetMinVesselAnisotropy(0.9);
  returnParameters->SetMaxVesselAnisotropy(0.9);
  returnParameters->SetMinVesselBending(0.1);
  returnParameters->SetMaxVesselBending(0.3);
  returnParameters->SetMinVesselRadiusInMillimeters(0.5);
  returnParameters->SetMaxVesselRadiusInMillimeters(4);
  returnParameters->SetMinVesselScattering(15);
  returnParameters->SetMaxVesselScattering(15);
  returnParameters->SetMinVesselZOrigin(2.2);
  returnParameters->SetMaxVesselZOrigin(4);
  returnParameters->SetVesselBifurcationFrequency(5000);
  returnParameters->SetRandomizePhysicalProperties(false);
  returnParameters->SetSkinThicknessInMillimeters(0);
  returnParameters->SetUseRngSeed(false);
  returnParameters->SetVoxelSpacingInCentimeters(0.06);
  returnParameters->SetXDim(70);
  returnParameters->SetYDim(100);
  returnParameters->SetZDim(100);
  return returnParameters;
}

TissueGeneratorParameters::Pointer CreateMultivessel_19_10_17_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(12);
  returnParameters->SetMinBackgroundAbsorption(0.1);
  returnParameters->SetMaxBackgroundAbsorption(0.1);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateNewRandomlyDivergingDirectionVector);
  returnParameters->SetDoPartialVolume(true);
  returnParameters->SetMinNumberOfVessels(1);
  returnParameters->SetMaxNumberOfVessels(7);
  returnParameters->SetMinVesselAbsorption(2);
  returnParameters->SetMaxVesselAbsorption(8);
  returnParameters->SetMinVesselAnisotropy(0.9);
  returnParameters->SetMaxVesselAnisotropy(0.9);
  returnParameters->SetMinVesselBending(0.1);
  returnParameters->SetMaxVesselBending(0.3);
  returnParameters->SetMinVesselRadiusInMillimeters(0.5);
  returnParameters->SetMaxVesselRadiusInMillimeters(4);
  returnParameters->SetMinVesselScattering(15);
  returnParameters->SetMaxVesselScattering(15);
  returnParameters->SetMinVesselZOrigin(2.2);
  returnParameters->SetMaxVesselZOrigin(4);
  returnParameters->SetVesselBifurcationFrequency(5000);
  returnParameters->SetRandomizePhysicalProperties(false);
  returnParameters->SetSkinThicknessInMillimeters(0);
  returnParameters->SetUseRngSeed(false);
  returnParameters->SetVoxelSpacingInCentimeters(0.03);
  returnParameters->SetXDim(140);
  returnParameters->SetYDim(200);
  returnParameters->SetZDim(180);
  return returnParameters;
}

TissueGeneratorParameters::Pointer CreateSinglevessel_19_10_17_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(12);
  returnParameters->SetMinBackgroundAbsorption(0.1);
  returnParameters->SetMaxBackgroundAbsorption(0.1);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateNewRandomlyDivergingDirectionVector);
  returnParameters->SetDoPartialVolume(true);
  returnParameters->SetMinNumberOfVessels(1);
  returnParameters->SetMaxNumberOfVessels(1);
  returnParameters->SetMinVesselAbsorption(2);
  returnParameters->SetMaxVesselAbsorption(8);
  returnParameters->SetMinVesselAnisotropy(0.9);
  returnParameters->SetMaxVesselAnisotropy(0.9);
  returnParameters->SetMinVesselBending(0.1);
  returnParameters->SetMaxVesselBending(0.3);
  returnParameters->SetMinVesselRadiusInMillimeters(0.5);
  returnParameters->SetMaxVesselRadiusInMillimeters(4);
  returnParameters->SetMinVesselScattering(15);
  returnParameters->SetMaxVesselScattering(15);
  returnParameters->SetMinVesselZOrigin(2.2);
  returnParameters->SetMaxVesselZOrigin(4);
  returnParameters->SetVesselBifurcationFrequency(5000);
  returnParameters->SetRandomizePhysicalProperties(false);
  returnParameters->SetSkinThicknessInMillimeters(0);
  returnParameters->SetUseRngSeed(false);
  returnParameters->SetVoxelSpacingInCentimeters(0.03);
  returnParameters->SetXDim(140);
  returnParameters->SetYDim(200);
  returnParameters->SetZDim(180);
  return returnParameters;
}

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
  parser.setTitle("Mitk Tissue Batch Generator");
  parser.setDescription("Creates in silico tissue in batch processing and automatically calculates fluence values for the central slice of the volume.");
  parser.setContributor("Computer Assisted Medical Interventions, DKFZ");

  // how should arguments be prefixed
  parser.setArgumentPrefix("--", "-");
  // add each argument, unless specified otherwise each argument is optional
  // see mitkCommandLineParser::addArgument for more information
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
  unsigned int iterationNumber = 0;

  while (true)
  {
    auto parameters = CreateBaselineHB_13_02_18_Parameters();
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

    std::string savePath = input.saveFolderPath + "input/BaselineHB_" + input.identifyer +
            "_" + std::to_string(iterationNumber) + ".nrrd";
    mitk::IOUtil::Save(resultTissue->ConvertToMitkImage(), savePath);
    std::string outputPath = input.saveFolderPath + "output/BaselineHB_" + input.identifyer +
            "_" + std::to_string(iterationNumber) + "/";

    if (!itksys::SystemTools::FileIsDirectory(outputPath))
    {
      itksys::SystemTools::MakeDirectory(outputPath);
    }

    outputPath = outputPath + "Fluence_BaselineHB_" + input.identifyer + "_" + std::to_string(iterationNumber);

    MITK_INFO(input.verbose) << "Simulating fluence..";
    for(double yo = -1.8; yo <= 1.81; yo=yo+0.12)
    {
        std::string yo_string = std::to_string(round(yo*100)/100.0);
        int result = -4;
        if(!input.probePath.empty())
            result = std::system(std::string(input.exePath + " -i " + savePath + " -o " +
                                             (outputPath + "_yo" + yo_string + ".nrrd") +
                                             " -yo " + yo_string + " -p " + input.probePath +
                                             " -n 100000000").c_str());
        else
            result = std::system(std::string(input.exePath + " -i " + savePath + " -o " +
                                             (outputPath + "_yo" + yo_string + ".nrrd") +
                                             " -yo " + yo_string + " -n 100000000").c_str());
        MITK_INFO << "yo: " << yo_string << ": " << result;
    }

    MITK_INFO(input.verbose) << "Simulating fluence..[Done]";

    iterationNumber++;
  }
}
