/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPANoiseGenerator.h"
#include "mitkPAIOUtil.h"
#include "mitkPAExceptions.h"
#include <random>
#include <chrono>
#include <mitkCommon.h>

void mitk::pa::NoiseGenerator::ApplyNoiseModel(mitk::pa::Volume::Pointer image, double detectorNoise, double speckleNoise)
{
  if (detectorNoise < 0 || speckleNoise < 0)
    throw mitk::pa::InvalidInputException("detectorNoise must be >= 0 and speckleNoise must be >= 0");

  if (detectorNoise < mitk::eps && speckleNoise < mitk::eps)
    return;

  std::mt19937 rng;
  std::random_device randomDevice;
  if (randomDevice.entropy() > mitk::eps)
  {
    rng.seed(randomDevice());
  }
  else
  {
    rng.seed(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
  }
  std::normal_distribution<> detector(detectorNoise / 2, detectorNoise / 2);
  std::normal_distribution<> speckle(1, speckleNoise);

  unsigned int negativecounter = 0;

  double* data = image->GetData();

  for (unsigned int x = 0, xLength = image->GetXDim(); x < xLength; x++)
    for (unsigned int y = 0, yLength = image->GetYDim(); y < yLength; y++)
      for (unsigned int z = 0, zLength = image->GetZDim(); z < zLength; z++)
      {
        double additiveNoise = detector(rng);

        double multiplicativeNoise = speckle(rng);

        double newSignal = (data[image->GetIndex(x, y, z)] + additiveNoise)*multiplicativeNoise;

        if (newSignal <= mitk::eps)
        {
          newSignal = mitk::eps;
          negativecounter++;
        }

        data[image->GetIndex(x, y, z)] = newSignal;
      }
}
