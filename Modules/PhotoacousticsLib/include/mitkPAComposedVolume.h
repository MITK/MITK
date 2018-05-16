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

#ifndef MITKPHOTOACOUSTICCOMPOSEDVOLUME_H
#define MITKPHOTOACOUSTICCOMPOSEDVOLUME_H

#include <MitkPhotoacousticsLibExports.h>

#include <mitkPAInSilicoTissueVolume.h>
#include <mitkPAFluenceYOffsetPair.h>
#include <vector>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    /**
     * @brief The ComposedVolume class provides the means to systematically collect nrrd files that correspond to
     * different simulation slices of the same InSilicoTissueVolume.
     *
     * An instance of this class is needed for the SlicedVolumeGenerator
     */
    class MITKPHOTOACOUSTICSLIB_EXPORT ComposedVolume : public itk::Object
    {
    public:
      mitkClassMacroItkParent(ComposedVolume, itk::Object)
        mitkNewMacro1Param(Self, InSilicoTissueVolume::Pointer)

        /**
         * @brief fluenceYOffsetPair
         *
         * @param nrrdFile path to the nrrd file on hard drive
         */
        void AddSlice(mitk::pa::FluenceYOffsetPair::Pointer fluenceYOffsetPair);

      /**
       * @brief GetNumberOfFluenceComponents
       * @return the number of fluence components encapsuled by this ComposedVolume.
       */
      int GetNumberOfFluenceComponents();

      /**
       * @brief GetFluenceValue
       * @param fluenceComponent
       * @param x
       * @param y
       * @param z
       * @return the fluence value for a specific fluence component index at the given 3D location.
       */
      double GetFluenceValue(int fluenceComponent, int x, int y, int z);

      /**
       * @brief GetYOffsetForFluenceComponentInPixels
       * @param fluenceComponent
       * @return the y-offset value for a given fluence component index.
       */
      int GetYOffsetForFluenceComponentInPixels(int fluenceComponent);

      void Sort();

      itkGetMacro(GroundTruthVolume, InSilicoTissueVolume::Pointer);

    protected:
      ComposedVolume(InSilicoTissueVolume::Pointer groundTruthVolume);
      ~ComposedVolume() override;

    private:
      int m_FluenceComponents;
      InSilicoTissueVolume::Pointer m_GroundTruthVolume;
      std::vector<mitk::pa::FluenceYOffsetPair::Pointer> m_FluenceValues;
    };
  }
}

#endif // MITKPHOTOACOUSTICCOMPOSEDVOLUME_H
