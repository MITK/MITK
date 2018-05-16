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

#ifndef MITKPHOTOACOUSTIC3DVOLUMEMANIPULATOR_H
#define MITKPHOTOACOUSTIC3DVOLUMEMANIPULATOR_H

#include <mitkCommon.h>
#include <MitkPhotoacousticsLibExports.h>
#include "mitkPAVolume.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT VolumeManipulator final
    {
    public:

      /**
      * @brief ThresholdImage applies a binary threshold filter to this image.
      * @param threshold
      */
      static void ThresholdImage(mitk::pa::Volume::Pointer image, double threshold);

      /**
      * @brief Multiplies the image with a given factor
      * @param factor
      */
      static void MultiplyImage(mitk::pa::Volume::Pointer image, double factor);

      static void GaussianBlur3D(mitk::pa::Volume::Pointer paVolume, double sigma);

      static void Log10Image(mitk::pa::Volume::Pointer image);

    private:
      VolumeManipulator();
      virtual ~VolumeManipulator();
    };
  }
}

#endif // MITKPHOTOACOUSTIC3DVOLUMEMANIPULATOR_H
