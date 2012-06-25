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

#ifndef __mitkDiffusionFunctionCollection_h_
#define __mitkDiffusionFunctionCollection_h_


#include "MitkDiffusionImagingExports.h"


namespace mitk{

class MitkDiffusionImaging_EXPORT sh
{
public:
static double factorial(int number);
static void Cart2Sph(double x, double y, double z, double* cart);
static double legendre0(int l);
static double spherical_harmonic(int m,int l,double theta,double phi, bool complexPart);
static double Yj(int m, int k, double theta, double phi);

};
}

#endif //__mitkDiffusionFunctionCollection_h_

