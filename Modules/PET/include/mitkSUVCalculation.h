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

#ifndef MITKSUVCALCULATION_H
#define MITKSUVCALCULATION_H

#include "mitkNumericConstants.h"
#include "MitkPETExports.h"

namespace mitk
{
   /**Computes the SUV body weight scale factor.
      @param injectedActivity Activity injected in [Bq]
      @param bodyweight Weight of the subject in [kg]
      @param decayTime Time between injection and measurement in [s]
      @param halfLife Halflife of the used nuclide in [s]*/
  double MITKPET_EXPORT computeSUVbwScaleFactor(double injectedActivity, double bodyweight, double decayTime, double halfLife);

   /**Computes the SUV body weighted for the passed value.
   @param value Value that should be scaled by the SUVbw factor to get the body weighted SUV.
   @param injectedActivity Activity injected in [Bq]
   @param bodyweight Weight of the subject in [kg]
   @param decayTime Time between injection and measurement in [s]
   @param halfLife Halflife of the used nuclide in [s]*/
  double MITKPET_EXPORT computeSUVbw(double value, double injectedActivity, double bodyweight, double decayTime, double halfLife);
};


#endif // PETSUVCALCULATION_H
