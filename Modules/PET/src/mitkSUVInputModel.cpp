/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <mitkBaseProperty.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkExceptionMacro.h>
#include <mitkIPropertyProvider.h>
#include <mitkSUVInputModel.h>

namespace
{
  std::string TrimAndUpper(const std::string &s)
  {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
      return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    std::string trimmed = s.substr(first, last - first + 1);
    std::transform(trimmed.begin(),
                   trimmed.end(),
                   trimmed.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return trimmed;
  }

  std::string ReadDICOMString(const mitk::IPropertyProvider *provider, const mitk::DICOMTagPath &path)
  {
    return mitk::GetFirstDICOMValueAsString(provider, path);
  }

  // Read a named simple property as a string. Used for the named
  // properties the PET DICOM reader lifts out of vendor private blocks
  // (e.g. mitk.pet.PhilipsSUVScale); those bypass DICOMTagPath because
  // they have no creator-string-aware path representation in MITK.
  std::string ReadNamedString(const mitk::IPropertyProvider *provider, const std::string &name)
  {
    auto prop = provider->GetConstProperty(name);
    if (prop.IsNull())
      return {};
    return prop->GetValueAsString();
  }

  bool ParsePositiveDouble(const std::string &raw, double &outValue)
  {
    if (raw.empty())
      return false;
    char *end = nullptr;
    errno = 0;
    const double v = std::strtod(raw.c_str(), &end);
    if (raw.c_str() == end)
      return false;
    if (!std::isfinite(v) || v <= 0.0)
      return false;
    outValue = v;
    return true;
  }

  // Resolve (0054,1006) SUV Type into a SUV variant. Per the IBSI-SUV
  // spec the Units tag carries only the unit (g/mL, cm^2/mL); the
  // specific variant lives in (0054,1006). When the tag is absent or
  // empty, IBSI prescribes BW as the default. The standard SUVType
  // value LBM (Morgan) is explicitly called out by IBSI as obsolete and
  // not covered by the DROs; we refuse it rather than silently picking
  // a different LBM formula.
  //
  // Returns std::nullopt when the tag is absent or empty so the caller
  // can apply the Units-implied default.
  std::optional<mitk::SUVVariant> ParseSUVTypeProperty(const mitk::IPropertyProvider *provider)
  {
    const std::string raw = ReadDICOMString(provider, mitk::DICOMTagPath(0x0054, 0x1006));
    const std::string v = TrimAndUpper(raw);
    if (v.empty())
      return std::nullopt;
    if ("BW" == v)
      return mitk::SUVVariant::BW;
    if ("LBMJANMA" == v)
      return mitk::SUVVariant::LBM_Janmahasatian;
    if ("LBMJAMES128" == v)
      return mitk::SUVVariant::LBM_James128;
    if ("IBW" == v)
      return mitk::SUVVariant::IBW;
    if ("BSA" == v)
      return mitk::SUVVariant::BSA;
    if ("LBM" == v)
    {
      mitkThrowException(mitk::UnsupportedPETUnitsException)
        << "(0054,1006) SUV Type = 'LBM' (Morgan) is obsolete per the "
           "IBSI-SUV recommendations and not supported. Use 'LBMJANMA' "
           "or 'LBMJAMES128' to specify the formula explicitly.";
    }
    mitkThrowException(mitk::UnsupportedPETUnitsException) << "(0054,1006) SUV Type = '" << raw
                                                           << "' is not supported. Recognised values: BW, LBMJANMA, "
                                                              "LBMJAMES128, IBW, BSA.";
  }
} // namespace

mitk::SUVInputModel mitk::ClassifyPETInput(const IPropertyProvider *provider, DICOMReadPolicy /*policy*/)
{
  if (nullptr == provider)
  {
    mitkThrow() << "ClassifyPETInput: provider is null.";
  }

  const std::string rawUnits = ReadDICOMString(provider, DICOMTagPath(0x0054, 0x1001));
  const std::string units = TrimAndUpper(rawUnits);
  if (units.empty())
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "(0054,1001) Units is required for SUV computation but is absent.";
  }

  SUVInputModel model;

  if ("BQML" == units)
  {
    model.semantics = SUVPixelSemantics::ActivityConcentration;
    model.activityScale = 1.0;
    return model;
  }

  if ("GML" == units)
  {
    // Per IBSI-SUV: Units=GML carries only the unit [g/mL]; the actual
    // pre-normalized SUV variant lives in (0054,1006) SUV Type.
    // Default (tag absent/empty): SUVbw.
    const auto suvType = ParseSUVTypeProperty(provider);
    const SUVVariant src = suvType.value_or(SUVVariant::BW);
    if (src == SUVVariant::BSA)
    {
      mitkThrowException(InvalidDICOMPropertyValueException)
        << "(0054,1001) Units = 'GML' is inconsistent with "
           "(0054,1006) SUV Type = 'BSA' (BSA SUV uses Units = 'CM2ML').";
    }
    model.semantics = SUVPixelSemantics::PrenormalizedSUV;
    model.sourceVariant = src;
    model.prenormScale = 1.0;
    return model;
  }

  if ("CM2ML" == units)
  {
    // Pre-computed SUVbsa. The IBSI-SUV spec couples CM2ML strictly to
    // SUV Type = BSA; reject any other SUVType as inconsistent.
    const auto suvType = ParseSUVTypeProperty(provider);
    if (suvType.has_value() && suvType.value() != SUVVariant::BSA)
    {
      mitkThrowException(InvalidDICOMPropertyValueException) << "(0054,1001) Units = 'CM2ML' is only valid with "
                                                                "(0054,1006) SUV Type = 'BSA'.";
    }
    model.semantics = SUVPixelSemantics::PrenormalizedSUV;
    model.sourceVariant = SUVVariant::BSA;
    model.prenormScale = 1.0;
    return model;
  }

  if ("CNTS" == units)
  {
    const std::string manuf = TrimAndUpper(ReadDICOMString(provider, DICOMTagPath(0x0008, 0x0070)));
    const bool isPhilips = (std::string::npos != manuf.find("PHILIPS"));
    if (!isPhilips)
    {
      mitkThrowException(UnsupportedPETUnitsException) << "(0054,1001) Units = 'CNTS' requires a Philips manufacturer "
                                                          "with one of the documented private scale factors. "
                                                          "Manufacturer (0008,0070) = '"
                                                       << manuf << "' is not Philips.";
    }

    // Look for the lifted Philips private factors. The lift is performed
    // by mitk::BaseDICOMReaderService when reading PET DICOM with
    // (7053,xx00) "Philips PET Private Group" present.
    const std::string suvScaleStr = ReadNamedString(provider, "mitk.pet.PhilipsSUVScale");
    const std::string activityScaleStr = ReadNamedString(provider, "mitk.pet.PhilipsActivityScale");

    double suvScale = 0.0;
    double actScale = 0.0;
    const bool haveSuv = ParsePositiveDouble(suvScaleStr, suvScale);
    const bool haveActivity = ParsePositiveDouble(activityScaleStr, actScale);

    if (haveSuv)
    {
      // pixel * SUV-scale yields SUVbw directly.
      model.semantics = SUVPixelSemantics::PrenormalizedSUV;
      model.sourceVariant = SUVVariant::BW;
      model.prenormScale = suvScale;
      return model;
    }
    if (haveActivity)
    {
      // pixel * activity-scale yields [Bq/mL]; standard SUV pipeline applies.
      model.semantics = SUVPixelSemantics::ActivityConcentration;
      model.activityScale = actScale;
      return model;
    }

    mitkThrowException(MissingPhilipsPETScaleException)
      << "(0054,1001) Units = 'CNTS' on Philips PET data requires either "
         "the Philips SUV-scale factor (7053,xx00) or activity-scale factor "
         "(7053,xx09); neither was found on the input.";
  }

  mitkThrowException(UnsupportedPETUnitsException) << "(0054,1001) Units = '" << rawUnits
                                                   << "' is not supported by the SUV pipeline. Supported values: "
                                                      "BQML, GML, CM2ML, CNTS (Philips with private scale factor).";
}
