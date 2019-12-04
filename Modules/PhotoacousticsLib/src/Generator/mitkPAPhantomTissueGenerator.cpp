/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPAPhantomTissueGenerator.h"
#include "mitkPAVector.h"
#include "mitkPAVolumeManipulator.h"

mitk::pa::InSilicoTissueVolume::Pointer mitk::pa::PhantomTissueGenerator::GeneratePhantomData(
  TissueGeneratorParameters::Pointer parameters)
{
  MITK_DEBUG << "Initializing Empty Volume";

  const double RESAMPLING_FACTOR = 2;

  if (parameters->GetDoPartialVolume())
  {
    parameters->SetXDim(parameters->GetXDim() * RESAMPLING_FACTOR);
    parameters->SetYDim(parameters->GetYDim() * RESAMPLING_FACTOR);
    parameters->SetZDim(parameters->GetZDim() * RESAMPLING_FACTOR);
    parameters->SetVesselBifurcationFrequency(parameters->GetVesselBifurcationFrequency() * RESAMPLING_FACTOR);
    parameters->SetVoxelSpacingInCentimeters(parameters->GetVoxelSpacingInCentimeters() / RESAMPLING_FACTOR);
  }

  parameters->SetVesselBifurcationFrequency(10000);

  std::mt19937 randomNumberGenerator;
  std::random_device randomDevice;
  if (parameters->GetUseRngSeed())
  {
    randomNumberGenerator.seed(parameters->GetRngSeed());
  }
  else
  {
    if (randomDevice.entropy() > 0.1)
    {
      randomNumberGenerator.seed(randomDevice());
    }
    else
    {
      randomNumberGenerator.seed(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    }
  }

  auto generatedVolume = mitk::pa::InSilicoTissueVolume::New(parameters, &randomNumberGenerator);

  const unsigned int NUMER_OF_VESSELS = 5;
  const double START_DEPTH_IN_CM = 2.10;
  const double START_X_IN_CM = 0.76;
  const double RADIUS_IN_MM = 0.5;
  const double INCREMENT_XZ_IN_CM = 0.50;
  double ABSORPTION_PER_CM = parameters->GetMinVesselAbsorption();

  generatedVolume->AddIntProperty("numberOfVessels", NUMER_OF_VESSELS);
  generatedVolume->AddIntProperty("bifurcationFrequency", parameters->GetVesselBifurcationFrequency());

  for (unsigned int vesselNumber = 0; vesselNumber < NUMER_OF_VESSELS; vesselNumber++)
  {
    Vector::Pointer initialPosition = Vector::New();
    Vector::Pointer initialDirection = Vector::New();

    double initialRadius = RADIUS_IN_MM / parameters->GetVoxelSpacingInCentimeters() / 10;
    std::stringstream radiusString;
    radiusString << "vessel_" << vesselNumber + 1 << "_radius";
    generatedVolume->AddDoubleProperty(radiusString.str(), initialRadius);

    double absorptionCoefficient = ABSORPTION_PER_CM;
    std::stringstream absorptionString;
    absorptionString << "vessel_" << vesselNumber + 1 << "_absorption";
    generatedVolume->AddDoubleProperty(absorptionString.str(), absorptionCoefficient);

    double bendingFactor = 0;
    std::stringstream bendingString;
    bendingString << "vessel_" << vesselNumber + 1 << "_bendingFactor";
    generatedVolume->AddDoubleProperty(bendingString.str(), bendingFactor);

    double vesselScattering = 15;
    std::stringstream scatteringString;
    scatteringString << "vessel_" << vesselNumber + 1 << "_scattering";
    generatedVolume->AddDoubleProperty(scatteringString.str(), vesselScattering);

    double vesselAnisotropy = 0.9;
    std::stringstream anisotropyString;
    anisotropyString << "vessel_" << vesselNumber + 1 << "_anisotropy";
    generatedVolume->AddDoubleProperty(anisotropyString.str(), vesselAnisotropy);

    /*The vessel tree shall start at one of the 4 sides of the volume.
    * The vessels will always be completely contained in the volume
    * when starting to meander.
    * They will meander in a direction perpendicular to the
    * plane they started from (within the limits of the
    * DIRECTION_VECTOR_INITIAL_VARIANCE)
    */

    double zposition = (START_DEPTH_IN_CM + (vesselNumber*INCREMENT_XZ_IN_CM)) / parameters->GetVoxelSpacingInCentimeters();

    double xposition = (START_X_IN_CM + (vesselNumber*INCREMENT_XZ_IN_CM)) / parameters->GetVoxelSpacingInCentimeters();

    initialPosition->Randomize(xposition, xposition, 0, 0, zposition, zposition, &randomNumberGenerator);
    initialDirection->Randomize(0, 0, 1, 1, 0, 0, &randomNumberGenerator);

    initialDirection->Normalize();
    MITK_INFO << initialPosition->GetElement(0) << " | " << initialPosition->GetElement(1) << " | " << initialPosition->GetElement(2);
    MITK_INFO << initialDirection->GetElement(0) << " | " << initialDirection->GetElement(1) << " | " << initialDirection->GetElement(2);

    VesselProperties::Pointer vesselParams = VesselProperties::New();
    vesselParams->SetDirectionVector(initialDirection);
    vesselParams->SetPositionVector(initialPosition);
    vesselParams->SetRadiusInVoxel(initialRadius);
    vesselParams->SetAbsorptionCoefficient(absorptionCoefficient);
    vesselParams->SetScatteringCoefficient(vesselScattering);
    vesselParams->SetAnisotopyCoefficient(vesselAnisotropy);
    vesselParams->SetBifurcationFrequency(parameters->GetVesselBifurcationFrequency());
    vesselParams->SetDoPartialVolume(parameters->GetDoPartialVolume());

    VesselTree::Pointer vesselTree = VesselTree::New(vesselParams);

    while (!vesselTree->IsFinished())
    {
      vesselTree->Step(generatedVolume, parameters->GetCalculateNewVesselPositionCallback(), bendingFactor, &randomNumberGenerator);
    }
  }

  if (parameters->GetDoPartialVolume())
  {
    VolumeManipulator::RescaleImage(generatedVolume, (1.0 / RESAMPLING_FACTOR));
    parameters->SetXDim(parameters->GetXDim() / RESAMPLING_FACTOR);
    parameters->SetYDim(parameters->GetYDim() / RESAMPLING_FACTOR);
    parameters->SetZDim(parameters->GetZDim() / RESAMPLING_FACTOR);
    parameters->SetVesselBifurcationFrequency(parameters->GetVesselBifurcationFrequency() / RESAMPLING_FACTOR);
    parameters->SetVoxelSpacingInCentimeters(parameters->GetVoxelSpacingInCentimeters() * RESAMPLING_FACTOR);
  }

  generatedVolume->FinalizeVolume();

  return generatedVolume;
}

mitk::pa::PhantomTissueGenerator::PhantomTissueGenerator()
{
}

mitk::pa::PhantomTissueGenerator::~PhantomTissueGenerator()
{
}
