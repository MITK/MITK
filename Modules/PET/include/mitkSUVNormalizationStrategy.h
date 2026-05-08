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
   * \li \c BW                - body-weight-normalized SUV (SUVbw). Output [g/mL].
   * \li \c LBM_Janmahasatian - lean-body-mass-normalized SUV (SUVlbm) using
   *                            the Janmahasatian (2005) formula. Output [g/mL].
   *                            IBSI-SUV-recommended LBM formula.
   * \li \c LBM_James128      - lean-body-mass-normalized SUV using the
   *                            James (1976) "James 128" formula. Output [g/mL].
   * \li \c IBW               - ideal-body-weight-normalized SUV using the
   *                            Devine (1974) formula. Output [g/mL].
   *                            Sex-specific.
   * \li \c BSA               - body-surface-area-normalized SUV (SUVbsa) using
   *                            the DuBois & DuBois (1916) formula. Output [cm^2/mL].
   *
   * \remark LBM_Janmahasatian, LBM_James128 and IBW depend on patient sex.
   *         The strategies in this header are sex-closed: they accept
   *         \c Sex::Male and \c Sex::Female only. \c Sex::Other is
   *         resolved upstream (typically by SUVImageFilter) under
   *         \c DICOMReadPolicy and never reaches the strategy.
   */
  enum class SUVVariant
  {
    BW,
    LBM_Janmahasatian,
    LBM_James128,
    IBW,
    BSA
  };

  /**
   * \brief Inputs available to a SUV normalization strategy.
   *
   * Aggregates all patient measurements that any built-in strategy may
   * require. Each strategy validates only the fields it consumes:
   *
   * \li BodyWeightStrategy:                 \c bodyWeightKg.
   * \li LeanBodyMassJanmahasatianStrategy:  \c bodyWeightKg, \c heightM, \c sex.
   * \li LeanBodyMassJames128Strategy:       \c bodyWeightKg, \c heightM, \c sex.
   * \li IdealBodyWeightStrategy:            \c heightM, \c sex.
   * \li BodySurfaceAreaStrategy:            \c bodyWeightKg, \c heightM.
   *
   * \remark Sex-specific strategies are sex-closed: they accept
   *         \c Sex::Male and \c Sex::Female only and throw
   *         \c MissingSUVInputException if \c sex is \c Sex::Other or
   *         absent. The IBSI-SUV mean-of-M-and-F adaptation for
   *         ambiguous sex (both \c Sex::Other and \c sex unset /
   *         unknown) is applied upstream by the caller (typically
   *         SUVImageFilter) under \c DICOMReadPolicy::Lenient, and
   *         refused with \c AmbiguousPatientSexAdaptationRefusedException
   *         under \c DICOMReadPolicy::Strict.
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
     *        is missing or non-finite, or if \c sex is \c Sex::Other
     *        for a sex-specific strategy (\c Sex::Other must be resolved
     *        upstream; see SUVNormalizationInputs).
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
   *
   * Reference: Strauss LG, Conti PS. "The applications of PET in clinical
   *            oncology." J Nucl Med 1991; 32:623-648.
   */
  class MITKPET_EXPORT BodyWeightStrategy : public SUVNormalizationStrategy
  {
  public:
    double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const override;
    SUVVariant Variant() const override { return SUVVariant::BW; }
  };

  /**
   * \brief Lean-body-mass-normalized SUV strategy using the Janmahasatian
   *        (2005) formula (SUVlbm-Janma).
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
   *
   * IBSI-SUV-recommended LBM formula.
   *
   * Reference: Janmahasatian S, Duffull SB, Ash S, Ward LC, Byrne NM,
   *            Green B. "Quantification of lean bodyweight."
   *            Clin Pharmacokinet 2005; 44(10):1051-1065.
   *
   * \pre \c inputs.sex is \c Sex::Male or \c Sex::Female. \c Sex::Other
   *      throws \c MissingSUVInputException; resolve it upstream.
   */
  class MITKPET_EXPORT LeanBodyMassJanmahasatianStrategy : public SUVNormalizationStrategy
  {
  public:
    double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const override;
    SUVVariant Variant() const override { return SUVVariant::LBM_Janmahasatian; }
  };

  /**
   * \brief Lean-body-mass-normalized SUV strategy using the James (1976)
   *        "James 128" formula (SUVlbm-James128).
   *
   * \code
   * LBM_male   [kg] = 1.10 * W - 0.0128 * W^2 / H^2
   * LBM_female [kg] = 1.07 * W - 0.0148 * W^2 / H^2
   * \endcode
   *
   * with \c W = body weight in [kg] and \c H = height in [m].
   *
   * Returns LBM expressed in grams (\c LBM_kg * 1000). Combined with a
   * [Bq/mL] PET image and a [Bq] dose, the resulting SUV has units
   * [g/mL].
   *
   * Reference: James WPT. "Research on Obesity." London: HMSO; 1976.
   *
   * \pre \c inputs.sex is \c Sex::Male or \c Sex::Female. \c Sex::Other
   *      throws \c MissingSUVInputException; resolve it upstream.
   */
  class MITKPET_EXPORT LeanBodyMassJames128Strategy : public SUVNormalizationStrategy
  {
  public:
    double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const override;
    SUVVariant Variant() const override { return SUVVariant::LBM_James128; }
  };

  /**
   * \brief Ideal-body-weight-normalized SUV strategy (SUVibw),
   *        Sugawara (1999) sex-specific formula.
   *
   * \code
   * IBW_male   [kg] = 48.0 + 1.06 * (H_cm - 152)
   * IBW_female [kg] = 45.5 + 0.91 * (H_cm - 152)
   * \endcode
   *
   * with \c H_cm = height in cm. Below 152 cm the formula yields values
   * below the constant; the strategy returns the formula's value
   * verbatim and does not clamp, leaving such inputs visible to the
   * caller.
   *
   * Returns IBW expressed in grams (\c IBW_kg * 1000). Combined with a
   * [Bq/mL] PET image and a [Bq] dose, the resulting SUV has units
   * [g/mL].
   *
   * Reference: Sugawara Y, Zasadny KR, Neuhoff AW, Wahl RL.
   *            "Reevaluation of the standardized uptake value for FDG:
   *             variations with body weight and methods for correction."
   *            Radiology 1999; 213(2):521-525.
   *            doi:10.1148/radiology.213.2.r99nv37521.
   *
   * \remark This formula is also the one mandated by the IBSI-SUV
   *         benchmark "Comprehensive Manual for SUV Computation and
   *         Digital Reference Objects for its Verification"
   *         (Suppl. Fig. 1 / SUV Type = "IBW"), which adopts Sugawara's
   *         table verbatim. It differs from classic Devine 1974
   *         (which uses 50.0 / 2.3 with H in inches and is not
   *         sex-differentiated in the slope).
   *
   * \pre \c inputs.sex is \c Sex::Male or \c Sex::Female. \c Sex::Other
   *      throws \c MissingSUVInputException; resolve it upstream.
   */
  class MITKPET_EXPORT IdealBodyWeightStrategy : public SUVNormalizationStrategy
  {
  public:
    double ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const override;
    SUVVariant Variant() const override { return SUVVariant::IBW; }
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
   *
   * Reference: DuBois D, DuBois EF. "A formula to estimate the
   *            approximate surface area if height and weight be known."
   *            Arch Intern Med 1916; 17:863-871.
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
