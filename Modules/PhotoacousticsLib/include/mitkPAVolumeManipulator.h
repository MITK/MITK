/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPHOTOACOUSTIC3DVOLUMEMANIPULATOR_H
#define MITKPHOTOACOUSTIC3DVOLUMEMANIPULATOR_H

#include <mitkCommon.h>
#include <MitkPhotoacousticsLibExports.h>
#include "mitkPAVolume.h"
#include "mitkPAInSilicoTissueVolume.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT VolumeManipulator final
    {
    public:

      /**
      * @brief ThresholdImage applies a binary threshold filter to this image.
      * @param threshold
      * @param image
      */
      static void ThresholdImage(Volume::Pointer image, double threshold);

      /**
      * @brief Multiplies the image with a given factor
      * @param factor
      * @param image
      */
      static void MultiplyImage(Volume::Pointer image, double factor);

      /**
      * @brief applies a Gaussian blur to an image
      * @param sigma
      * @param paVolume
      */
      static void GaussianBlur3D(Volume::Pointer paVolume, double sigma);

      static void Log10Image(Volume::Pointer image);

      static void RescaleImage(InSilicoTissueVolume::Pointer image, double ratio);

      static Volume::Pointer RescaleImage(Volume::Pointer image, double ratio, double sigma);

    private:
      VolumeManipulator();
      virtual ~VolumeManipulator();
    };
  }
}

#endif // MITKPHOTOACOUSTIC3DVOLUMEMANIPULATOR_H
