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
      mitkClassMacroItkParent(VesselMeanderStrategy, itk::LightObject)
        itkFactorylessNewMacro(Self)

        /**
         * @brief CalculateNewPositionInStraightLine calculates the new position by just following the direction vector.
         * @param position
         * @param direction
         * @param bendingFactor
         */
        void CalculateNewPositionInStraightLine(Vector::Pointer position, Vector::Pointer direction, double bendingFactor, std::mt19937* rng);

      /**
       * @brief CalculateRandomlyDivergingPosition calculates the new position by modifying the direction vector randomly,
       * proportional to the selected bendingFactor. This means, that the vessels will bend in each expansion step,
       * if bendingFactor > 0.
       *
       * @param position
       * @param direction
       * @param bendingFactor
       */
      void CalculateRandomlyDivergingPosition(Vector::Pointer position, Vector::Pointer direction, double bendingFactor, std::mt19937* rng);

    protected:
      VesselMeanderStrategy();
      ~VesselMeanderStrategy() override;

      const double RANDOMIZATION_PERCENTAGE = 0.4;
    };
  }
}
#endif // MITKVESSELMEANDERSTRATEGY_H
