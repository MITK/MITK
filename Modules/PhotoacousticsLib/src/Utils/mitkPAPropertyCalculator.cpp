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

#include "mitkPAPropertyCalculator.h"
// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>

mitk::pa::PropertyCalculator::Properties mitk::pa::PropertyCalculator::
CalculatePropertyForSpecificWavelength(
  TissueType tissueType, int wavelength, double bloodOxygenInFraction)
{
  Properties returnValue;

  if (!m_Valid)
  {
    mitkThrow() << "PhotoacousticPropertyGenerator was not loaded properly.";
    return returnValue;
  }

  double bloodOxygenation = bloodOxygenInFraction;
  double bloodVolumeFraction;
  double waterVolumeFraction;
  double fatVolumeFraction;
  double melanosomesVolumeFraction;
  double musp500;
  double fray;
  double bmie;
  double g;

  switch (tissueType)
  {
  case AIR:
    returnValue.mua = 1e-4;
    returnValue.mus = 1.0;
    returnValue.g = 1.0;
    return returnValue;
  case BLOOD:
    bloodVolumeFraction = 1.0;
    waterVolumeFraction = 0.95;
    fatVolumeFraction = 0.02;
    melanosomesVolumeFraction = 0;
    musp500 = 10;
    fray = 0;
    bmie = 1;
    g = 0.9;
    break;
  case EPIDERMIS:
    bloodVolumeFraction = 0;
    bloodOxygenation = 0;
    waterVolumeFraction = 0.75;
    fatVolumeFraction = 0.01;
    melanosomesVolumeFraction = 0.02;
    musp500 = 40;
    fray = 0;
    bmie = 1;
    g = 0.9;
    break;
  case FAT:
    bloodVolumeFraction = 0.01;
    bloodOxygenation = 0.75;
    waterVolumeFraction = 0.5;
    fatVolumeFraction = 0.6;
    melanosomesVolumeFraction = 0;
    musp500 = 17.47;
    fray = 0.2;
    bmie = 0.5;
    g = 0.9;
    break;
  case STANDARD_TISSUE:
  default:
    bloodVolumeFraction = 0.02;
    bloodOxygenation = 0.75;
    waterVolumeFraction = 0.8;
    fatVolumeFraction = 0.05;
    melanosomesVolumeFraction = 0;
    musp500 = 25;
    fray = 0.25;
    bmie = 1;
    g = 0.9;
    break;
  }

  // We want the reduced scattering coefficient directly.
  double musp = musp500 * (fray * pow(wavelength / 500.0, -4.0) + ((1 - fray) * pow(wavelength / 500.0, -bmie)));
  returnValue.mus = musp;
  returnValue.mus = 15;//musp;

  double mua = bloodVolumeFraction*bloodOxygenation*m_SpectralLibMap[MapType::OXYGENATED][wavelength] +
    bloodVolumeFraction*(1 - bloodOxygenation)*m_SpectralLibMap[MapType::DEOXYGENATED][wavelength] +
    waterVolumeFraction*m_SpectralLibMap[MapType::WATER][wavelength] +
    fatVolumeFraction*m_SpectralLibMap[MapType::FATTY][wavelength] +
    melanosomesVolumeFraction*m_SpectralLibMap[MapType::MELANIN][wavelength];

  returnValue.mua = mua;

  returnValue.g = g;

  return returnValue;
}

mitk::pa::PropertyCalculator::PropertyCalculator()
{
  us::ModuleResource resource = us::GetModuleContext()->GetModule()->GetResource("spectralLIB.dat");

  if (resource.IsValid())
  {
    us::ModuleResourceStream resourceStream(resource);
    std::string line;
    int wavelength = 300;
    int counter = 0;
    while (std::getline(resourceStream, line, '\t'))
    {
      int currentLineIdx = counter % 6;
      if (currentLineIdx == 0)
        wavelength = std::stoi(line);
      else
      {
        std::istringstream lineStream(line);
        double tempDouble;
        lineStream >> tempDouble;
        m_SpectralLibMap[currentLineIdx][wavelength] = tempDouble;
      }
      ++counter;
    }
  }
  else
  {
    m_Valid = false;
  }

  m_Valid = true;
}

mitk::pa::PropertyCalculator::~PropertyCalculator()
{
  m_SpectralLibMap.clear();
  m_Valid = false;
}
