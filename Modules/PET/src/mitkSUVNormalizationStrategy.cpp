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
}

double mitk::BodyWeightStrategy::ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.bodyWeightKg, "bodyWeightKg");
  return inputs.bodyWeightKg * 1000.0;
}

double mitk::LeanBodyMassStrategy::ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.bodyWeightKg, "bodyWeightKg");
  RequirePositive(inputs.heightM,      "heightM");
  if (!inputs.sex.has_value())
  {
    mitkThrowException(mitk::MissingSUVInputException)
      << "SUV normalization input 'sex' is required for SUVlbm "
         "(Janmahasatian formula).";
  }

  // Janmahasatian (2005). BMI in [kg/m^2], W in [kg]. Returns LBM in [kg].
  const double w   = inputs.bodyWeightKg;
  const double h   = inputs.heightM;
  const double bmi = w / (h * h);

  const double lbmKg = (inputs.sex.value() == Sex::Male)
    ? (9270.0 * w) / (6680.0 + 216.0 * bmi)
    : (9270.0 * w) / (8780.0 + 244.0 * bmi);

  return lbmKg * 1000.0;
}

double mitk::BodySurfaceAreaStrategy::ComputeScaleNumerator(const SUVNormalizationInputs& inputs) const
{
  RequirePositive(inputs.bodyWeightKg, "bodyWeightKg");
  RequirePositive(inputs.heightM,      "heightM");

  // DuBois & DuBois (1916): BSA [m^2] = 0.007184 * W^0.425 * H_cm^0.725
  const double w    = inputs.bodyWeightKg;
  const double hCm  = inputs.heightM * 100.0;
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
    case SUVVariant::BW:  return std::make_unique<BodyWeightStrategy>();
    case SUVVariant::LBM: return std::make_unique<LeanBodyMassStrategy>();
    case SUVVariant::BSA: return std::make_unique<BodySurfaceAreaStrategy>();
  }
  mitkThrow() << "MakeSUVNormalizationStrategy: unhandled SUVVariant value.";
}
