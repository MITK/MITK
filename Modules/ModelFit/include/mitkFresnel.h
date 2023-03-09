/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFresnel_h
#define mitkFresnel_h

/****************************************************************************
 *              fresnel.h -
 *  Calculation of Fresnel integrals by expansion to Chebyshev series
 *  Expansions are taken from the book
 *  Y.L. Luke. Mathematical functions and their approximations.
 *  Ìoscow, "Mir", 1980. PP. 145-149 (Russian edition)
 ****************************************************************************
 */

namespace mitk
{

  /* fresnel_c(x) - Fresnel Cosine Integral
   * C(x)=fresnel_c(x)=\dint\limits_{0}^{x}\cos (\frac{\pi}{2}t^{2})dt
   */
  double fresnel_c(double x);

  /* fresnel_s(x) - Fresnel Sine Integral
   * S(x)=fresnel_s(x)=\dint\limits_{0}^{x}\sin (\frac{\pi}{2}t^{2})dt
   */
  double fresnel_s(double x);

  /* Additional functions*/
  /* fresnel_c2(x)
   * fresnel_c2(x)=fresnel_c(x*sqrt(2/pi))=
   * = \sqrt{\frac{2}{\pi }}\dint\limits_{0}^{x}\cos (t^{2})dt
   */
  double fresnel_c2(double x);

  /* fresnel_s2(x)
   * fresnel_s2(x)=fresnel_s(x*sqrt(2/pi))=
   * = \sqrt{\frac{2}{\pi }}\dint\limits_{0}^{x}\sin (t^{2})dt
   */
  double fresnel_s2(double x);
}

#endif
