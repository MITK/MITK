/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkDiffusionFunctionCollection_h_
#define __mitkDiffusionFunctionCollection_h_

template<class T>
class vnl_vector;

namespace mitk{

namespace sh
{

double factorial(int number);
void Cart2Sph(double x, double y, double z, double* cart);
double legendre0(int l);
double spherical_harmonic(int m,int l,double theta,double phi, bool complexPart);
double Yj(int m, int k, double theta, double phi);

}

namespace vnl_function
{

template<class CurrentValue, class WntValue>
vnl_vector<WntValue> element_cast (vnl_vector<CurrentValue> const& v1);

}
}

#endif //__mitkDiffusionFunctionCollection_h_

