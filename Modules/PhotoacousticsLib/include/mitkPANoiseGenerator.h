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

#ifndef MITKPHOTOACOUSTICNOISEGENERATOR_H
#define MITKPHOTOACOUSTICNOISEGENERATOR_H

#include "MitkPhotoacousticsLibExports.h"

#include "mitkPAVolume.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT NoiseGenerator final
    {
    public:

      /**
       * @brief ApplyNoiseModel
       * Applies noise to an image.
       * @param image the image to apply the noise to
       * @param detectorNoise >= 0
       * @param speckleNoise >= 1
       */
      static void ApplyNoiseModel(mitk::pa::Volume::Pointer image, double detectorNoise, double speckleNoise);

    private:
      NoiseGenerator();
      ~NoiseGenerator() {};
    };
  }
}

#endif // MITKPHOTOACOUSTICNOISEGENERATOR_H
