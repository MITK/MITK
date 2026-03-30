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

#ifndef mitkSUVCalculation_h
#define mitkSUVCalculation_h

#include <mitkNumericConstants.h>
#include <MitkPETExports.h>

namespace mitk
{
  /**
   * \brief Compute the SUV body weight (SUVbw) scale factor.
   *
   * Calculates the multiplicative scale factor used to convert raw PET pixel values to
   * body-weight-normalized Standardized Uptake Values (SUVbw). The formula accounts for
   * radioactive decay between injection and measurement time:
   *
   * \code
   * factor = (bodyweight * 1000) / (injectedActivity * 2^(-decayTime / halfLife))
   * \endcode
   *
   * \param[in] injectedActivity Activity injected in [Bq].
   * \param[in] bodyweight Weight of the subject in [kg].
   * \param[in] decayTime Time between injection and measurement in [s].
   * \param[in] halfLife Half-life of the used radionuclide in [s].
   * \return The SUVbw scale factor (dimensionless, [g/Bq] effectively).
   *
   * \sa computeSUVbw, SUVbwFunctorPolicy, HALFLIFECONSTANTS
   */
  double MITKPET_EXPORT computeSUVbwScaleFactor(double injectedActivity, double bodyweight, double decayTime, double halfLife);

  /**
   * \brief Compute the body-weight-normalized SUV (SUVbw) for a single value.
   *
   * Multiplies the given raw PET pixel value by the SUVbw scale factor to produce
   * the body-weight-normalized Standardized Uptake Value.
   *
   * \param[in] value Raw PET pixel value to be converted (typically in [Bq/ml]).
   * \param[in] injectedActivity Activity injected in [Bq].
   * \param[in] bodyweight Weight of the subject in [kg].
   * \param[in] decayTime Time between injection and measurement in [s].
   * \param[in] halfLife Half-life of the used radionuclide in [s].
   * \return The computed SUVbw value [g/ml].
   *
   * \sa computeSUVbwScaleFactor, SUVbwFunctorPolicy
   */
  double MITKPET_EXPORT computeSUVbw(double value, double injectedActivity, double bodyweight, double decayTime, double halfLife);
};


#endif // PETSUVCALCULATION_H
