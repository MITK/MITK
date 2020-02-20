/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPHOTOACOUSTICSLICEDVOLUMEGENERATOR_H
#define MITKPHOTOACOUSTICSLICEDVOLUMEGENERATOR_H

#include <MitkPhotoacousticsLibExports.h>

#include <mitkPAComposedVolume.h>
#include <mitkPAVolume.h>
#include <mitkImage.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SlicedVolumeGenerator : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(SlicedVolumeGenerator, itk::LightObject);
      mitkNewMacro4Param(Self, int, bool,
        mitk::pa::Volume::Pointer, bool);

      mitk::pa::Volume::Pointer GetSlicedFluenceImageFromComposedVolume(
        mitk::pa::ComposedVolume::Pointer composedVolume);

      mitk::pa::Volume::Pointer GetSlicedSignalImageFromComposedVolume(
        mitk::pa::ComposedVolume::Pointer composedVolume);

      mitk::pa::Volume::Pointer GetSlicedGroundTruthImageFromComposedVolume(
        mitk::pa::ComposedVolume::Pointer composedVolume);

    protected:
      SlicedVolumeGenerator(int centralYSlice, bool precorrect,
        mitk::pa::Volume::Pointer precorrectionVolume, bool inverse);
      ~SlicedVolumeGenerator() override;

      int m_CentralYSlice;
      bool m_Precorrect;
      bool m_Inverse;
      mitk::pa::Volume::Pointer m_PrecorrectionVolume;
    };
  }
}
#endif // MITKPHOTOACOUSTICSLICEDVOLUMEGENERATOR_H
