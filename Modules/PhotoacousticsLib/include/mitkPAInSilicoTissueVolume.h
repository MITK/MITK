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

#ifndef MITKPHOTOACOUSTICVOLUME_H
#define MITKPHOTOACOUSTICVOLUME_H

#include <MitkPhotoacousticsLibExports.h>
#include <mitkImage.h>
#include <mitkProperties.h>
#include <mitkPATissueGeneratorParameters.h>
#include <mitkPAVolume.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT InSilicoTissueVolume : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(InSilicoTissueVolume, itk::LightObject)
        mitkNewMacro1Param(Self, TissueGeneratorParameters::Pointer)

        enum SegmentationType
      {
        AIR = -1,
        BACKGROUND = 0,
        VESSEL = 1,
        FAT = 2,
        SKIN = 3
      };

      /**
       * @brief ConvertToMitkImage
       * @return a pointer to an mitk image containing this volume.
       */
      mitk::Image::Pointer ConvertToMitkImage();

      /**
       * @brief SetVolumeValues sets the values for aborption, scattering and anisotropy at the specified voxel location.
       *
       * @param x
       * @param y
       * @param z
       * @param absorption
       * @param scattering
       * @param anisotropy
       * @param segmentType
       */
      void SetVolumeValues(int x, int y, int z, double absorption, double scattering, double anisotropy, SegmentationType segmentType = SegmentationType::BACKGROUND);

      /**
       * @brief IsInsideVolume
       *
       * @param x
       * @param y
       * @param z
       * @return true if the voxel location is inside the volume
       */
      bool IsInsideVolume(int x, int y, int z);

      /**
       * @brief AddDoubleProperty adds a persistent property to the volume, which will be exported to the mitk image.
       *
       * @param label
       * @param value
       */
      void AddDoubleProperty(std::string label, double value);

      /**
       * @brief AddIntProperty adds a persistent property to the volume, which will be exported to the mitk image.
       *
       * @param label
       * @param value
       */
      void AddIntProperty(std::string label, int value);

      Volume::Pointer GetAbsorptionVolume();
      Volume::Pointer GetScatteringVolume();
      Volume::Pointer GetAnisotropyVolume();
      Volume::Pointer GetSegmentationVolume();

      void FinalizeVolume();
      itkGetMacro(TissueParameters, TissueGeneratorParameters::Pointer);
      itkGetMacro(TDim, unsigned int);

      static InSilicoTissueVolume::Pointer New(mitk::pa::Volume::Pointer absorptionVolume,
        Volume::Pointer scatteringVolume,
        Volume::Pointer anisotropyVolume,
        Volume::Pointer segmentationVolume,
        TissueGeneratorParameters::Pointer tissueParameters,
        mitk::PropertyList::Pointer propertyList);

    protected:
      InSilicoTissueVolume(TissueGeneratorParameters::Pointer parameters);
      InSilicoTissueVolume(Volume::Pointer absorptionVolume,
        Volume::Pointer scatteringVolume,
        Volume::Pointer anisotropyVolume,
        Volume::Pointer segmentationVolume,
        TissueGeneratorParameters::Pointer tissueParameters,
        mitk::PropertyList::Pointer propertyList);
      ~InSilicoTissueVolume() override;

      mitk::pa::Volume::Pointer m_AbsorptionVolume;
      mitk::pa::Volume::Pointer m_ScatteringVolume;
      mitk::pa::Volume::Pointer m_AnisotropyVolume;
      mitk::pa::Volume::Pointer m_SegmentationVolume;

      TissueGeneratorParameters::Pointer m_TissueParameters;

      unsigned int m_TDim;

      void RandomizeTissueCoefficients(long rngSeed, bool useRngSeed, double percentage);

      mitk::PropertyList::Pointer m_PropertyList;

    private:
      void FillZLayer(int x, int y, double startIdx, double endIdx,
        double absorption, double scattering, double anisotropy,
        SegmentationType segmentationType);
      void AddSkinAndAirLayers();
      void UpdatePropertyList();
    };
  }
}
#endif // MITKPHOTOACOUSTICVOLUME_H
