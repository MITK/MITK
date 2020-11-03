/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
