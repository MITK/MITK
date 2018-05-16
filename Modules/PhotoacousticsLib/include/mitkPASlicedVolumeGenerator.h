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
