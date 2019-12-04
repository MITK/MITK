/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPASimulationBatchGenerator.h"
#include <mitkIOUtil.h>

#include <iomanip>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

mitk::pa::SimulationBatchGenerator::SimulationBatchGenerator()
{
}

mitk::pa::SimulationBatchGenerator::~SimulationBatchGenerator()
{
}

std::string mitk::pa::SimulationBatchGenerator::GetVolumeNumber(
  SimulationBatchGeneratorParameters::Pointer parameters)
{
  std::string volumeNumber = std::to_string(parameters->GetVolumeIndex());
  volumeNumber = std::string((3 - volumeNumber.length()), '0') + volumeNumber;
  return volumeNumber;
}

std::string mitk::pa::SimulationBatchGenerator::GetOutputFolderName(
  SimulationBatchGeneratorParameters::Pointer parameters)
{
  return (parameters->GetTissueName() + GetVolumeNumber(parameters));
}

std::string mitk::pa::SimulationBatchGenerator::CreateBatchSimulationString(
  SimulationBatchGeneratorParameters::Pointer parameters)
{
  std::string outputFolderName = GetOutputFolderName(parameters);
  std::string savePath = outputFolderName + ".nrrd";
  std::stringstream batchstring;
  for (double d = parameters->GetYOffsetLowerThresholdInCentimeters();
    d <= parameters->GetYOffsetUpperThresholdInCentimeters() + 1e-5;
    d += parameters->GetYOffsetStepInCentimeters())
  {
    batchstring << parameters->GetBinaryPath() << " -p PROBE_DESIGN.xml -i " << savePath << " -o " << outputFolderName << "/"
      << parameters->GetTissueName() << GetVolumeNumber(parameters) << "_yo" << round(d * 100) / 100 << ".nrrd" << " -yo " << round(d * 100) / 100 << " -n "
      << parameters->GetNumberOfPhotons() << "\n";
  }
  return batchstring.str();
}

void mitk::pa::SimulationBatchGenerator::WriteBatchFileAndSaveTissueVolume(
  SimulationBatchGeneratorParameters::Pointer parameters,
  mitk::Image::Pointer tissueVolume)
{
  std::string outputFolderName = parameters->GetNrrdFilePath() + GetOutputFolderName(parameters);
  std::string savePath = outputFolderName + ".nrrd";
  mitk::IOUtil::Save(tissueVolume, savePath);

  std::string filenameAllSimulation = "simulate_all";
#ifdef _WIN32
  mkdir(outputFolderName.c_str());
  filenameAllSimulation += ".bat";
#else
  mkdir(outputFolderName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  filenameAllSimulation += ".sh";
#endif

  std::ofstream fileAllSimulation(parameters->GetNrrdFilePath() + "/" + filenameAllSimulation, std::ios_base::app);
  if (fileAllSimulation.is_open())
  {
    fileAllSimulation << CreateBatchSimulationString(parameters);
    fileAllSimulation.close();
  }
}
