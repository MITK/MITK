/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSUVNormalizationStrategy.h>

#include <cmath>

namespace
{
  void RequireFinite(double value, const char* fieldName)
  {
    if (!std::isfinite(value))
    {
      mitkThrowException(mitk::MissingSUVInputException)
        << "SUV normalization input '" << fieldName
        << "' is missing or not a finite number.";
    }
  }

  void RequirePositive(double value, const char* fieldName)
  {
    RequireFinite(value, fieldName);
    if (value <= 0.0)
    {
      mitkThrowException(mitk::MissingSUVInputException)
        << "SUV normalization input '" << fieldName
        << "' must be positive (got " << value << ").";
    }
  }

  // Sex-closed contract: the sex-specific strategies accept Male and
  // Female only. Sex::Other is an upstream-resolved IBSI adaptation
  // (mean of M and F) and must not reach the strategy.
  void RequireConcreteSex(const std::optional<mitk::Sex>& sex,
                          const char*                     variantName)
  {
    if (!sex.has_value())
    {
      mitkThrowException(mitk::MissingSUVInputException)
        << "SUV normalization input 'sex' is required for " << variantName << ".";
    }
    if (mitk::Sex::Other == sex.value())
    {
      mitkThrowException(mitk::MissingSUVInputException)
        << "SUV normalization input 'sex' must be Male or Female for " << variantName
        << ". The IBSI mean-of-M-and-F adaptation for Sex::Other is "
           "policy-gated upstream and must not reach the strategy.";
    }
  }
}

double mitk::BodyWeightStrategy::ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.bodyWeightKg, "bodyWeightKg");
  return inputs.bodyWeightKg * 1000.0;
}

double mitk::LeanBodyMassJanmahasatianStrategy::ComputeScaleNumerator(
  const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.bodyWeightKg, "bodyWeightKg");
  RequirePositive(inputs.heightM,      "heightM");
  RequireConcreteSex(inputs.sex, "SUVlbm (Janmahasatian)");

  // Janmahasatian (2005). BMI in [kg/m^2], W in [kg]. Returns LBM in [kg].
  const double w   = inputs.bodyWeightKg;
  const double h   = inputs.heightM;
  const double bmi = w / (h * h);

  const double lbmKg = (Sex::Male == inputs.sex.value())
    ? (9270.0 * w) / (6680.0 + 216.0 * bmi)
    : (9270.0 * w) / (8780.0 + 244.0 * bmi);

  return lbmKg * 1000.0;
}

double mitk::LeanBodyMassJames128Strategy::ComputeScaleNumerator(
  const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.bodyWeightKg, "bodyWeightKg");
  RequirePositive(inputs.heightM,      "heightM");
  RequireConcreteSex(inputs.sex, "SUVlbm (James 128)");

  // James (1976), "James 128" formulation expressed in kg / m:
  //   LBM_male   [kg] = 1.10 * W - 0.0128 * W^2 / H^2
  //   LBM_female [kg] = 1.07 * W - 0.0148 * W^2 / H^2
  const double w  = inputs.bodyWeightKg;
  const double h  = inputs.heightM;
  const double w2 = w * w;
  const double h2 = h * h;

  const double lbmKg = (Sex::Male == inputs.sex.value())
    ? 1.10 * w - 0.0128 * w2 / h2
    : 1.07 * w - 0.0148 * w2 / h2;

  return lbmKg * 1000.0;
}

double mitk::IdealBodyWeightStrategy::ComputeScaleNumerator(
  const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.heightM, "heightM");
  RequireConcreteSex(inputs.sex, "SUVibw (Sugawara)");

  // Sugawara (1999), sex-specific IBW formula (also adopted verbatim by
  // the IBSI-SUV benchmark for SUV Type = "IBW"):
  //   IBW_male   [kg] = 48.0 + 1.06 * (H_cm - 152)
  //   IBW_female [kg] = 45.5 + 0.91 * (H_cm - 152)
  // Below 152 cm the formula yields values below the constant; we
  // return the formula value verbatim rather than clamping, leaving
  // such inputs visible to the caller.
  const double heightCm = inputs.heightM * 100.0;
  const double dH       = heightCm - 152.0;
  const double ibwKg    = (Sex::Male == inputs.sex.value())
    ? 48.0 + 1.06 * dH
    : 45.5 + 0.91 * dH;

  return ibwKg * 1000.0;
}

double mitk::BodySurfaceAreaStrategy::ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.bodyWeightKg, "bodyWeightKg");
  RequirePositive(inputs.heightM,      "heightM");

  // DuBois & DuBois (1916): BSA [m^2] = 0.007184 * W^0.425 * H_cm^0.725
  const double w     = inputs.bodyWeightKg;
  const double hCm   = inputs.heightM * 100.0;
  const double bsaM2 = 0.007184 * std::pow(w, 0.425) * std::pow(hCm, 0.725);

  // Convert to cm^2 so the SUV output unit is cm^2/mL when the PET
  // image is in Bq/mL and the dose is in Bq.
  return bsaM2 * 10000.0;
}

std::unique_ptr<mitk::SUVNormalizationStrategy>
mitk::MakeSUVNormalizationStrategy(SUVVariant variant)
{
  switch (variant)
  {
    case SUVVariant::BW:                return std::make_unique<BodyWeightStrategy>();
    case SUVVariant::LBM_Janmahasatian: return std::make_unique<LeanBodyMassJanmahasatianStrategy>();
    case SUVVariant::LBM_James128:      return std::make_unique<LeanBodyMassJames128Strategy>();
    case SUVVariant::IBW:               return std::make_unique<IdealBodyWeightStrategy>();
    case SUVVariant::BSA:               return std::make_unique<BodySurfaceAreaStrategy>();
  }
  mitkThrow() << "MakeSUVNormalizationStrategy: unhandled SUVVariant value.";
}
