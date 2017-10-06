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

using namespace mitk::pa;

TissueGeneratorParameters::Pointer CreateMultivessel_06_10_17_Parameters()
{
  auto returnParameters = TissueGeneratorParameters::New();
  returnParameters->SetAirThicknessInMillimeters(12);
  returnParameters->SetBackgroundAbsorption(0.1);
  returnParameters->SetBackgroundAnisotropy(0.9);
  returnParameters->SetBackgroundScattering(15);
  returnParameters->SetCalculateNewVesselPositionCallback(&VesselMeanderStrategy::CalculateRandomlyDivergingPosition);
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
  returnParameters->SetZDim(140);
  return returnParameters;
}

struct InputParameters
{
  std::string saveFolderPath;
  std::string identifyer;
  std::string exePath;
};

InputParameters parseInput(int argc, char* argv[])
{
  InputParameters input;

  return input;
}

int main(int argc, char * argv[])
{
  auto input = parseInput(argc, argv);
  unsigned int iterationNumber = 0;

  while (iterationNumber == 0)
  {
    auto parameters = CreateMultivessel_06_10_17_Parameters();
    auto resultTissue = InSilicoTissueGenerator::GenerateInSilicoData(parameters);

    std::string savePath = input.saveFolderPath + "/MultiVessel_" + input.identifyer + std::to_string(iterationNumber) + ".nrrd";
    mitk::IOUtil::Save(resultTissue->ConvertToMitkImage(), savePath);

    iterationNumber++;
  }
}
