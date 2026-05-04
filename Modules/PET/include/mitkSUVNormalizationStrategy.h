/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSUVNormalizationStrategy_h
#define mitkSUVNormalizationStrategy_h

#include <limits>
#include <memory>
#include <optional>

#include <mitkException.h>
#include <mitkExceptionMacro.h>

#include <mitkSUVCalculationHelper.h>

#include <MitkPETExports.h>

namespace mitk
{
  /**
   * \brief A required SUV normalization input is missing or invalid.
   *
   * Raised by SUVNormalizationStrategy::ComputeScaleNumerator when one
   * of the patient measurements required by the chosen variant is not
   * present in the SUVNormalizationInputs (e.g. height for SUVlbm, sex
   * for SUVlbm) or is not a finite number.
   *
   * The exception message names the missing/invalid field so callers
   * can render an actionable diagnostic.
   */
  class MITKPET_EXPORT MissingSUVInputException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(MissingSUVInputException, SUVHelperException);
  };

  /**
   * \brief Identifies a Standardized Uptake Value (SUV) variant.
   *
   * \li \c BW  - body-weight-normalized SUV (SUVbw). Output [g/mL].
   * \li \c LBM - lean-body-mass-normalized SUV (SUVlbm). Output [g/mL].
   * \li \c BSA - body-surface-area-normalized SUV (SUVbsa). Output [cm^2/mL].
   */
  enum class SUVVariant
  {
    BW,
    LBM,
    BSA
  };

  /**
   * \brief Inputs available to a SUV normalization strategy.
   *
   * Aggregates all patient measurements that any built-in strategy may
   * require. Each strategy validates only the fields it consumes:
   *
   * \li BodyWeightStrategy:       \c bodyWeightKg.
   * \li LeanBodyMassStrategy:     \c bodyWeightKg, \c heightM, \c sex.
   * \li BodySurfaceAreaStrategy:  \c bodyWeightKg, \c heightM.
   *
   * Unsupplied fields hold NaN (numeric) or have no value (\c sex);
   * the strategy throws MissingSUVInputException on first access of an
   * unsupplied required field.
   */
  struct MITKPET_EXPORT SUVNormalizationInputs
  {
    /** Body weight in [kg]. NaN if not supplied. */
    double             bodyWeightKg = std::numeric_limits<double>::quiet_NaN();
    /** Patient height in [m]. NaN if not supplied. */
    double             heightM      = std::numeric_limits<double>::quiet_NaN();
    /** Patient sex. Empty optional if not supplied. */
    std::optional<Sex> sex;
  };

  /**
   * \brief Strategy that computes the variant-specific scale numerator
   *        used by the SUV scale factor.
   *
   * The kernel formula (see computeSUVScaleFactor) is variant-agnostic:
   *
   * \code
   * factor = scaleNumerator / (injectedActivity * 2^(-decayTime / halfLife))
   * \endcode
   *
   * Each variant supplies a different \c scaleNumerator with a
   * different unit; the resulting SUV inherits those units multiplied
   * by the inverse of the input PET unit.
   *
   * Concrete strategies:
   *   - BodyWeightStrategy:       returns body weight [g], output [g/mL].
   *   - LeanBodyMassStrategy:     returns lean body mass [g], output [g/mL].
   *   - BodySurfaceAreaStrategy:  returns body surface area [cm^2],
   *                               output [cm^2/mL].
   *
   * \sa SUVVariant, SUVNormalizationInputs, computeSUVScaleFactor,
   *     SUVFunctorPolicy
   */
  class MITKPET_EXPORT SUVNormalizationStrategy
  {
  public:
    virtual ~SUVNormalizationStrategy() = default;

    /**
     * \brief Compute the variant-specific scale numerator.
     *
     * \param[in] inputs The patient measurements available to the strategy.
     * \return The scale numerator in the strategy's output unit (see
     *         class documentation for the per-variant unit).
     * \throw MissingSUVInputException if a required field of \p inputs
     *        is missing or non-finite.
     */
    virtual double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const = 0;

    /** \return The variant identifier this strategy implements. */
    virtual SUVVariant Variant() const = 0;
  };

  /**
   * \brief Body-weight-normalized SUV strategy (SUVbw).
   *
   * Returns \c bodyWeightKg * 1000 (i.e. body weight expressed in
   * grams). Combined with a [Bq/mL] PET image and a [Bq] dose, the
   * resulting SUV has units [g/mL].
   */
  class MITKPET_EXPORT BodyWeightStrategy : public SUVNormalizationStrategy
  {
  public:
    double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const override;
    SUVVariant Variant() const override { return SUVVariant::BW; }
  };

  /**
   * \brief Lean-body-mass-normalized SUV strategy (SUVlbm).
   *
   * Uses the Janmahasatian (2005) formula:
   *
   * \code
   * LBM_male   [kg] = (9270 * W) / (6680 + 216 * BMI)
   * LBM_female [kg] = (9270 * W) / (8780 + 244 * BMI)
   * \endcode
   *
   * with \c W = body weight in [kg] and \c BMI = W / H^2 (H in [m]).
   *
   * Returns LBM expressed in grams (\c LBM_kg * 1000). Combined with a
   * [Bq/mL] PET image and a [Bq] dose, the resulting SUV has units
   * [g/mL].
   */
  class MITKPET_EXPORT LeanBodyMassStrategy : public SUVNormalizationStrategy
  {
  public:
    double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const override;
    SUVVariant Variant() const override { return SUVVariant::LBM; }
  };

  /**
   * \brief Body-surface-area-normalized SUV strategy (SUVbsa).
   *
   * Uses the DuBois & DuBois (1916) formula:
   *
   * \code
   * BSA [m^2] = 0.007184 * W^0.425 * H_cm^0.725
   * \endcode
   *
   * with \c W = body weight in [kg] and \c H_cm = patient height
   * in [cm] (the input \c heightM is multiplied by 100 internally).
   *
   * Returns BSA expressed in [cm^2] (BSA_m2 * 10000). Combined with a
   * [Bq/mL] PET image and a [Bq] dose, the resulting SUV has units
   * [cm^2/mL].
   */
  class MITKPET_EXPORT BodySurfaceAreaStrategy : public SUVNormalizationStrategy
  {
  public:
    double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const override;
    SUVVariant Variant() const override { return SUVVariant::BSA; }
  };

  /**
   * \brief Construct the SUV normalization strategy for the given variant.
   *
   * \param[in] variant The SUV variant the strategy should implement.
   * \return A unique-owned strategy instance.
   */
  std::unique_ptr<SUVNormalizationStrategy> MITKPET_EXPORT
  MakeSUVNormalizationStrategy(SUVVariant variant);
}

#endif
