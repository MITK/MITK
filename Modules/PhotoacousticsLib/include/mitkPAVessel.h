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

#ifndef MITKVESSEL_H
#define MITKVESSEL_H

#include "mitkVector.h"
#include "mitkPAVesselMeanderStrategy.h"
#include "mitkPAInSilicoTissueVolume.h"
#include "mitkPAVector.h"
#include "mitkPAVesselProperties.h"

#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT Vessel : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(Vessel, itk::LightObject)
        mitkNewMacro1Param(Self, VesselProperties::Pointer)

        /**
         * Callback function definition of a VesselMeanderStrategy
         */
        typedef void (VesselMeanderStrategy::*CalculateNewVesselPositionCallback)
        (Vector::Pointer, Vector::Pointer, double, std::mt19937*);

      /**
       * @brief ExpandVessel makes this Vessel expand one step in its current direction.
       * After expanding, the vessel will draw itself into the given InSilicoTissueVolume.
       *
       * @param volume volume for the vessel to draw itself in
       * @param calculateNewPosition a callback function of the VesselMeanderStrategy class.
       * It is used to  calculate the final position after taking the step.
       * @param bendingFactor a metric of how much the Vessel should bend. If set to 0 the vessel will go in a straight line.
       */
      void ExpandVessel(mitk::pa::InSilicoTissueVolume::Pointer volume,
        CalculateNewVesselPositionCallback calculateNewPosition, double bendingFactor, std::mt19937* rng);

      /**
       * @brief CanBifurcate
       * @return true if the Vessel is ready to Bifurcate()
       */
      bool CanBifurcate();

      /**
       * @brief Bifurcate bifurcates this vessel into two new ones. Makes sure that the volume of the vessels stays the same.
       *
       * @return a new vessel split up from the current one.
       */
      Vessel::Pointer Bifurcate(std::mt19937* rng);

      /**
       * @brief IsFinished
       * @return true if the vessel cannot expand any further
       */
      bool IsFinished();

      itkGetConstMacro(VesselProperties, VesselProperties::Pointer);

    protected:
      Vessel(VesselProperties::Pointer parameters);
      ~Vessel() override;

    private:

      const double MINIMUM_VESSEL_RADIUS = 1;
      const double SCALING_FACTOR = 0.33;
      const double NEW_RADIUS_MINIMUM_RELATIVE_SIZE = 0.6;
      const double NEW_RADIUS_MAXIMUM_RELATIVE_SIZE = 0.8;

      void DrawVesselInVolume(Vector::Pointer toPosition, mitk::pa::InSilicoTissueVolume::Pointer volume);
      VesselProperties::Pointer m_VesselProperties;

      VesselMeanderStrategy::Pointer m_VesselMeanderStrategy;
      bool m_Finished;
      double m_WalkedDistance;

      std::uniform_real_distribution<> m_RangeDistribution;
      std::uniform_real_distribution<> m_SignDistribution;
      std::uniform_real_distribution<> m_RadiusRangeDistribution;

      int GetSign(std::mt19937* rng);
    };

    /**
    * @brief Equal A function comparing two vessels for beeing equal
    *
    * @param rightHandSide A vessel to be compared
    * @param leftHandSide A vessel to be compared
    * @param eps tolarence for comparison. You can use mitk::eps in most cases.
    * @param verbose flag indicating if the user wants detailed console output or not.
    * @return true, if all subsequent comparisons are true, false otherwise
    */
    MITKPHOTOACOUSTICSLIB_EXPORT bool Equal(const Vessel::Pointer leftHandSide, const Vessel::Pointer rightHandSide, double eps, bool verbose);
  }
}
#endif // MITKVESSEL_H
