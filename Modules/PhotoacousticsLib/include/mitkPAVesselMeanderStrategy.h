/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKVESSELMEANDERSTRATEGY_H
#define MITKVESSELMEANDERSTRATEGY_H

#include "mitkVector.h"
#include "mitkPAVector.h"

#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT VesselMeanderStrategy : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(VesselMeanderStrategy, itk::LightObject);
        itkFactorylessNewMacro(Self);

        /**
         * @brief CalculateNewPositionInStraightLine calculates the new position by just following the direction vector.
         * @param direction
         * @param bendingFactor
         */
        void CalculateNewDirectionVectorInStraightLine(Vector::Pointer direction, double bendingFactor, std::mt19937* rng);

      /**
       * @brief CalculateRandomlyDivergingPosition calculates the new position by modifying the direction vector randomly,
       * proportional to the selected bendingFactor. This means, that the vessels will bend in each expansion step,
       * if bendingFactor > 0.
       *
       * @param direction
       * @param bendingFactor
       */
      void CalculateNewRandomlyDivergingDirectionVector(Vector::Pointer direction, double bendingFactor, std::mt19937* rng);

    protected:
      VesselMeanderStrategy();
      ~VesselMeanderStrategy() override;

      const double RANDOMIZATION_PERCENTAGE = 0.4;
    };
  }
}
#endif // MITKVESSELMEANDERSTRATEGY_H
