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
   * \brief Compute the generic SUV scale factor.
   *
   * Calculates the multiplicative scale factor used to convert raw PET
   * pixel values to a Standardized Uptake Value (SUV). The formula
   * accounts for radioactive decay between injection and measurement
   * time:
   *
   * \code
   * factor = scaleNumerator / (injectedActivity * 2^(-decayTime / halfLife))
   * \endcode
   *
   * The SUV variant (SUVbw, SUVlbm, SUVbsa, ...) is encoded in the
   * \p scaleNumerator: callers obtain the variant-specific value (and
   * its units) from a SUVNormalizationStrategy. The math kernel itself
   * is variant-agnostic.
   *
   * \param[in] injectedActivity Activity injected in [Bq].
   * \param[in] scaleNumerator   Variant-specific normalization quantity
   *                             whose units determine the SUV output
   *                             units (e.g. body weight in [g] for
   *                             SUVbw / SUVlbm, body surface area in
   *                             [cm^2] for SUVbsa).
   * \param[in] decayTime        Time between injection and measurement
   *                             in [s].
   * \param[in] halfLife         Half-life of the used radionuclide in [s].
   * \return The SUV scale factor.
   *
   * \sa computeSUVbwScaleFactor, SUVFunctorPolicy, SUVNormalizationStrategy
   */
  double MITKPET_EXPORT computeSUVScaleFactor(double injectedActivity, double scaleNumerator, double decayTime, double halfLife);

  /**
   * \brief Compute the SUV body weight (SUVbw) scale factor.
   *
   * Backward-compatible wrapper around computeSUVScaleFactor() that
   * encodes the SUVbw normalization (\c bodyweight * 1000 expressed in
   * grams).
   *
   * \code
   * factor = (bodyweight * 1000) / (injectedActivity * 2^(-decayTime / halfLife))
   * \endcode
   *
   * \param[in] injectedActivity Activity injected in [Bq].
   * \param[in] bodyweight       Weight of the subject in [kg].
   * \param[in] decayTime        Time between injection and measurement in [s].
   * \param[in] halfLife         Half-life of the used radionuclide in [s].
   * \return The SUVbw scale factor (output [g/Bq]; applied to a
   *         [Bq/mL] image yields [g/mL]).
   *
   * \sa computeSUVbw, HALFLIFECONSTANTS
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
   * \sa computeSUVbwScaleFactor
   */
  double MITKPET_EXPORT computeSUVbw(double value, double injectedActivity, double bodyweight, double decayTime, double halfLife);
};


#endif
