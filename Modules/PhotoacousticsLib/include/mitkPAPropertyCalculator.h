/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPHOTOACOUSTICPROPERTYCALCULATOR_H
#define MITKPHOTOACOUSTICPROPERTYCALCULATOR_H

#include "MitkPhotoacousticsLibExports.h"

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT PropertyCalculator : public itk::LightObject
    {
    public:

      mitkClassMacroItkParent(PropertyCalculator, itk::LightObject);
        itkFactorylessNewMacro(Self);

      struct Properties
      {
        double mua;
        double mus;
        double g;
      };

      enum TissueType
      {
        AIR = 1,
        BLOOD = 2,
        EPIDERMIS = 3,
        FAT = 4,
        STANDARD_TISSUE = 5
      };

      enum ChromophoreType
      {
        OXYGENATED = 1,
        DEOXYGENATED = 2,
        WATER = 3,
        FATTY = 4,
        MELANIN = 5,
        ONEENDMEMBER = 6
      };

      double GetAbsorptionForWavelength(
        ChromophoreType chromophoreType, int wavelength);

      Properties CalculatePropertyForSpecificWavelength(
        TissueType tissueType, int wavelength, double oxygenSaturatonInFraction = 0);

    protected:
      PropertyCalculator();
      ~PropertyCalculator() override;

      bool m_Valid = false;

      std::map<int, std::map<int, double>> m_SpectralLibMap;

    };
  }
}

#endif // MITKPHOTOACOUSTICPROPERTYCALCULATOR_H
