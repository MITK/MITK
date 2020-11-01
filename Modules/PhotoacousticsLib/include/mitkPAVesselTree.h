/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKVESSELSTRUCTURE_H
#define MITKVESSELSTRUCTURE_H

//std includes
#include <vector>

//mitk includes
#include "mitkPAVessel.h"
#include "mitkPAInSilicoTissueVolume.h"
#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT VesselTree : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(VesselTree, itk::LightObject);
      mitkNewMacro1Param(Self, VesselProperties::Pointer);

      /**
        * @brief Step Performs a simulation step, in which all subvessels of this VesselTree are expanded.
        *
        * @param volume
        * @param calculateNewPosition
        * @param bendingFactor
        * @param rng
        */
      void Step(InSilicoTissueVolume::Pointer volume,
        Vessel::CalculateNewVesselPositionCallback calculateNewPosition,
        double bendingFactor, std::mt19937* rng);

      /**
       * @brief IsFinished
       * @return true if no subvessel can be expanded.
       */
      bool IsFinished();

      itkGetConstMacro(CurrentSubvessels, std::vector<Vessel::Pointer>*);

    protected:
      VesselTree(VesselProperties::Pointer initialProperties);
      ~VesselTree() override;

    private:
      std::vector<Vessel::Pointer>* m_CurrentSubvessels;
    };

    MITKPHOTOACOUSTICSLIB_EXPORT bool Equal(const VesselTree::Pointer leftHandSide, const VesselTree::Pointer rightHandSide, double eps, bool verbose);
  }
}
#endif // MITKVESSELSTRUCTURE_H
