/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <mitkBaseProperty.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkExceptionMacro.h>
#include <mitkIPropertyProvider.h>
#include <mitkSUVInputModel.h>
#include <mitkSlicedGeometry3D.h>

#include "mitkSUVEnhancedPETGuards.h"
#include "mitkSUVFunctionalGroupAccess.h"

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

  // UCUM unit codes are case-sensitive, so they are trimmed but never
  // uppercased the way TrimAndUpper does for the DICOM enumerations.
  std::string TrimAscii(const std::string &s)
  {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
      return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
  }

  std::string ReadDICOMString(const mitk::IPropertyProvider *provider, const mitk::DICOMTagPath &path)
  {
    return mitk::GetFirstDICOMValueAsString(provider, path);
  }

  bool ParseFiniteDouble(const std::string &raw, double &outValue)
  {
    if (raw.empty())
      return false;
    char *end = nullptr;
    errno = 0;
    const double v = std::strtod(raw.c_str(), &end);
    if (raw.c_str() == end || !std::isfinite(v))
      return false;
    outValue = v;
    return true;
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

  // Read a lifted vendor private numeric factor that the PET reader attaches
  // per-(timestep, slice). Philips emits a single series-level scale, but it
  // is stored on every frame; read all populated slots and require them to
  // agree so a malformed multi-bed export does not silently collapse to the
  // first frame (the sibling decay-datetime consumer also reads per-(t,s)).
  // Returns false when the property is absent or holds no usable value;
  // throws InvalidDICOMPropertyValueException when populated slots disagree.
  bool ReadUniformPositiveFactor(const mitk::IPropertyProvider *provider,
                                 const std::string &name,
                                 double &outValue)
  {
    auto baseProp = provider->GetConstProperty(name);
    if (baseProp.IsNull())
      return false;

    const auto *tsProp =
      dynamic_cast<const mitk::TemporoSpatialStringProperty *>(baseProp.GetPointer());
    if (nullptr == tsProp)
    {
      // Already collapsed to a single uniform value.
      return ParsePositiveDouble(baseProp->GetValueAsString(), outValue);
    }

    bool haveValue = false;
    double common = 0.0;
    for (const auto t : tsProp->GetAvailableTimeSteps())
    {
      for (const auto s : tsProp->GetAvailableSlices(t))
      {
        double v = 0.0;
        if (!ParsePositiveDouble(tsProp->GetValue(t, s, false, false), v))
          continue;
        if (!haveValue)
        {
          common = v;
          haveValue = true;
        }
        else if (std::fabs(v - common) >
                 1e-9 * std::max(std::fabs(v), std::fabs(common)))
        {
          mitkThrowException(mitk::InvalidDICOMPropertyValueException)
            << "Philips private scale factor '" << name << "' varies across "
               "frames (" << common << " vs " << v << "); the SUV pipeline "
               "treats it as a single series-level factor.";
        }
      }
    }
    if (!haveValue)
      return false;
    outValue = common;
    return true;
  }

  // Resolve (0054,1006) SUV Type into a SUV variant. Per the IBSI-SUV
  // spec the Units tag carries only the unit (g/mL, cm^2/mL); the
  // specific variant lives in (0054,1006). When the tag is absent or
  // empty, IBSI prescribes BW as the default. Every standard SUV Type is
  // accepted, LBM (Morgan) included: it is obsolete and no DRO covers it,
  // but it is convertible and refusing it would leave real data
  // unreadable.
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
    // "LBM" is the Morgan variant. The IBSI-SUV manual calls it obsolete
    // but lists it among the SUV Types that must be convertible, and data
    // carrying it exists regardless of what a reader would prefer.
    if ("LBM" == v)
      return mitk::SUVVariant::LBM_Morgan;
    mitkThrowException(mitk::UnsupportedPETUnitsException) << "(0054,1006) SUV Type = '" << raw
                                                           << "' is not supported. Recognised values: BW, LBM, "
                                                              "LBMJANMA, LBMJAMES128, IBW, BSA.";
  }
} // namespace


namespace
{
  // One RWVM item's unit, resolved onto the (semantics, variant) model the
  // rest of the pipeline speaks.
  struct EnhancedUnit
  {
    mitk::SUVPixelSemantics semantics = mitk::SUVPixelSemantics::ActivityConcentration;
    mitk::SUVVariant        variant   = mitk::SUVVariant::BW;
    // Preference when one frame offers several mappings: the IBSI-SUV manual
    // takes SUVbw first, then any other SUV type, then plain activity
    // concentration. Lower sorts first.
    int                     priority  = 2;

    bool operator==(const EnhancedUnit& other) const
    {
      return semantics == other.semantics && variant == other.variant;
    }
  };

  // UCUM code values the SUV pipeline can convert. Only Bq/ml and
  // g/ml{SUVbw} appear in the benchmark; the remaining SUV types are mapped
  // rather than evidenced, because refusing a unit DICOM defines and the
  // pipeline already models would be an arbitrary gap.
  std::optional<EnhancedUnit> MapUcumUnitCode(const std::string& rawCode)
  {
    // Case is not normalized away: UCUM is case-sensitive, and "g/ml{SUVbw}"
    // differs from "g/ml{SUVBW}" only in the annotation, which is free text.
    const std::string code = TrimAscii(rawCode);

    if ("Bq/ml" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::ActivityConcentration, mitk::SUVVariant::BW, 2};
    if ("g/ml{SUVbw}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::BW, 0};
    if ("g/ml{SUVlbm}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::LBM_Morgan, 1};
    if ("g/ml{SUVlbm(Janma)}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::LBM_Janmahasatian, 1};
    if ("g/ml{SUVlbm(James128)}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::LBM_James128, 1};
    if ("g/ml{SUVibw}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::IBW, 1};
    if ("cm2/ml{SUVbsa}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::BSA, 1};

    return std::nullopt;
  }

  // Index of the first sequence item a property name selects: for a Real
  // World Value Mapping attribute, which mapping item it belongs to. -1 when
  // the name selects no item.
  int MappingItemIndex(const std::string& propertyName)
  {
    const auto path = mitk::PropertyNameToDICOMTagPath(propertyName);
    for (mitk::DICOMTagPath::PathIndexType i = 0; i < path.Size(); ++i)
    {
      const auto& node = path.GetNode(i);
      if (mitk::DICOMTagPath::NodeInfo::NodeType::SequenceSelection == node.type)
      {
        return node.selection;
      }
    }
    return -1;
  }

  using Slot = std::pair<mitk::TimeStepType, mitk::DICOMProperty::IndexValueType>;

  std::string ValueAt(const mitk::BaseProperty* property, const Slot& slot)
  {
    return mitk::SUVFunctionalGroupAccess::ValueAt(property, slot.first, slot.second);
  }

  // The same slope reads as "4.0" through the Pixel Value Transformation
  // Sequence (a DS) and "4" through the Real World Value Mapping Sequence
  // (an FD), so the pairs are compared as numbers. A DS often carries only a
  // few significant digits of the FD it was written from, hence the relative
  // term; the absolute one lets a zero intercept equal a double residue. Both
  // stay far below the factor that separates a Bq/ml from an SUV mapping.
  bool NearlyEqual(double a, double b)
  {
    const double difference = std::fabs(a - b);
    return difference <= 1e-12 || difference <= 1e-4 * std::max(std::fabs(a), std::fabs(b));
  }

  std::string Describe(const Slot& slot)
  {
    return "timestep " + std::to_string(slot.first) + " slice " + std::to_string(slot.second);
  }

  // One Real World Value Mapping item's unit-code property.
  struct MappingCode
  {
    int                              item = -1;
    mitk::BaseProperty::ConstPointer property;
  };

  // What one slot's mappings say about the unit of the loaded pixel values.
  struct SlotUnit
  {
    Slot                        slot;
    bool                        anyMapping = false;
    // The best-ranked unit among the mappings whose slope and intercept
    // equal the Pixel Value Transformation the reader applied at this slot.
    std::optional<EnhancedUnit> unit;
    std::vector<std::string>    unknownCodes;
    std::vector<std::string>    unusableMappings;
    std::string                 appliedPair;
  };

  // Resolve one slot. The pipeline never applies a Real World Value Mapping;
  // the loaded values are the stored ones through the Pixel Value
  // Transformation, so a mapping describes them only when its pair equals
  // that transformation. Consistency therefore gates the manual's ranking
  // rather than following it: a Bq/ml mapping equal to the transformation
  // beside a g/ml{SUVbw} mapping that is not are two descriptions of the
  // same stored values, and only the first describes the loaded buffer.
  SlotUnit ResolveSlotUnit(const Slot& slot,
                           const std::vector<MappingCode>& codes,
                           const std::map<int, mitk::BaseProperty::ConstPointer>& mappingSlopes,
                           const std::map<int, mitk::BaseProperty::ConstPointer>& mappingIntercepts,
                           const mitk::BaseProperty* appliedSlope,
                           const mitk::BaseProperty* appliedIntercept)
  {
    SlotUnit result;
    result.slot = slot;

    // The reader applies the Pixel Value Transformation of the frame; where
    // the macro is absent it applied nothing, i.e. slope 1 and intercept 0.
    double pvtSlope = 1.0;
    double pvtIntercept = 0.0;
    const std::string rawSlope = (nullptr != appliedSlope) ? TrimAscii(ValueAt(appliedSlope, slot)) : "";
    const std::string rawIntercept =
      (nullptr != appliedIntercept) ? TrimAscii(ValueAt(appliedIntercept, slot)) : "";
    if (!rawSlope.empty() && !ParseFiniteDouble(rawSlope, pvtSlope))
    {
      mitkThrowException(mitk::InvalidDICOMPropertyValueException)
        << "(0028,1053) Rescale Slope '" << rawSlope << "' at " << Describe(slot)
        << " is not a finite number.";
    }
    if (!rawIntercept.empty() && !ParseFiniteDouble(rawIntercept, pvtIntercept))
    {
      mitkThrowException(mitk::InvalidDICOMPropertyValueException)
        << "(0028,1052) Rescale Intercept '" << rawIntercept << "' at " << Describe(slot)
        << " is not a finite number.";
    }
    result.appliedPair = (rawSlope.empty() ? "1" : rawSlope) + "/" + (rawIntercept.empty() ? "0" : rawIntercept);

    for (const auto& code : codes)
    {
      const std::string rawCode = TrimAscii(ValueAt(code.property, slot));
      if (rawCode.empty())
      {
        continue;
      }
      result.anyMapping = true;

      const auto unit = MapUcumUnitCode(rawCode);
      if (!unit.has_value())
      {
        result.unknownCodes.push_back(rawCode);
        continue;
      }

      // A mapping without slope and intercept carries a Real World Value LUT
      // instead, which nothing here applies.
      const auto slopeIt = mappingSlopes.find(code.item);
      const auto interceptIt = mappingIntercepts.find(code.item);
      const std::string mappingSlope =
        (slopeIt != mappingSlopes.end()) ? TrimAscii(ValueAt(slopeIt->second, slot)) : "";
      const std::string mappingIntercept =
        (interceptIt != mappingIntercepts.end()) ? TrimAscii(ValueAt(interceptIt->second, slot)) : "";
      double slope = 0.0;
      double intercept = 0.0;
      if (mappingSlope.empty() || mappingIntercept.empty() ||
          !ParseFiniteDouble(mappingSlope, slope) || !ParseFiniteDouble(mappingIntercept, intercept))
      {
        result.unusableMappings.push_back(rawCode + " (no slope/intercept)");
        continue;
      }
      if (!NearlyEqual(slope, pvtSlope) || !NearlyEqual(intercept, pvtIntercept))
      {
        result.unusableMappings.push_back(rawCode + " (" + mappingSlope + "/" + mappingIntercept + ")");
        continue;
      }

      if (!result.unit.has_value() || unit->priority < result.unit->priority)
      {
        result.unit = unit;
      }
    }
    return result;
  }

  mitk::SUVInputModel ModelForUnit(const EnhancedUnit& unit)
  {
    mitk::SUVInputModel model;
    model.semantics = unit.semantics;
    if (mitk::SUVPixelSemantics::PrenormalizedSUV == unit.semantics)
    {
      model.sourceVariant = unit.variant;
      model.prenormScale = 1.0;
    }
    else
    {
      model.activityScale = 1.0;
    }
    return model;
  }

  std::string Join(const std::vector<std::string>& parts)
  {
    std::string joined;
    for (const auto& part : parts)
    {
      joined += (joined.empty() ? "" : ", ") + part;
    }
    return joined;
  }

  // The manual marks Rescale Type a fallback because it is supposed to be
  // "US" for PET, so a unit hiding there is a departure from the standard
  // rather than the intended place to look.
  mitk::SUVInputModel ClassifyByRescaleType(const mitk::IPropertyProvider* provider,
                                            const mitk::BaseProperty* rescaleType,
                                            const std::set<Slot>& slots)
  {
    if (nullptr == rescaleType)
    {
      mitkThrowException(mitk::MissingDICOMPropertyException)
        << "Enhanced PET: the pixel unit could not be determined. Neither the "
           "Measurement Units Code Sequence (0040,08EA) inside the Real World "
           "Value Mapping Sequence nor (0028,1054) Rescale Type yielded a unit "
           "the SUV pipeline converts.";
    }

    std::string common;
    for (const auto& slot : slots)
    {
      const std::string units = TrimAndUpper(ValueAt(rescaleType, slot));
      if (units.empty())
      {
        mitkThrowException(mitk::MissingDICOMPropertyException)
          << "Enhanced PET: (0028,1054) Rescale Type is absent at " << Describe(slot)
          << " while other frames carry it.";
      }
      if (common.empty())
      {
        common = units;
      }
      else if (units != common)
      {
        mitkThrowException(mitk::EnhancedPETPerFrameVariationException)
          << "(0028,1054) Rescale Type names different units for different "
             "frames (" << common << " vs " << units << " at " << Describe(slot)
          << "). MITK carries one unit per image, so the input cannot be "
             "represented; it is not malformed.";
      }
    }

    mitk::SUVInputModel model;
    if ("BQML" == common)
    {
      model.semantics = mitk::SUVPixelSemantics::ActivityConcentration;
      model.activityScale = 1.0;
      return model;
    }
    if ("GML" == common)
    {
      const auto suvType = ParseSUVTypeProperty(provider);
      model.semantics = mitk::SUVPixelSemantics::PrenormalizedSUV;
      model.sourceVariant = suvType.value_or(mitk::SUVVariant::BW);
      model.prenormScale = 1.0;
      return model;
    }
    if ("CM2ML" == common)
    {
      model.semantics = mitk::SUVPixelSemantics::PrenormalizedSUV;
      model.sourceVariant = mitk::SUVVariant::BSA;
      model.prenormScale = 1.0;
      return model;
    }

    mitkThrowException(mitk::UnsupportedPETUnitsException)
      << "Enhanced PET: no usable Measurement Units Code Sequence, and "
         "(0028,1054) Rescale Type = '" << common << "' is not one of "
         "BQML, GML, CM2ML.";
  }
}

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
    // (7053,xx00) "Philips PET Private Group" present. Read per-(t,s) and
    // require uniformity rather than trusting the first frame.
    double suvScale = 0.0;
    double actScale = 0.0;
    const bool haveSuv = ReadUniformPositiveFactor(provider, "mitk.pet.PhilipsSUVScale", suvScale);
    const bool haveActivity = ReadUniformPositiveFactor(provider, "mitk.pet.PhilipsActivityScale", actScale);

    // The activity-concentration factor takes precedence. It yields Bq/mL and
    // then feeds the standard SUV pipeline, so the patient data that goes
    // into the normalization is the data this run was configured with --
    // including any override. The SUV-scale factor instead yields SUVbw
    // directly, baking in whatever weight the scanner held at acquisition
    // time, which cannot be corrected afterwards. Where an export carries
    // both, the recoverable path is the right one to take.
    //
    // ReadUniformPositiveFactor reports absent, empty and non-positive alike
    // as "not available", which is exactly the precedence condition.
    if (haveActivity)
    {
      // pixel * activity-scale yields [Bq/mL]; standard SUV pipeline applies.
      model.semantics = SUVPixelSemantics::ActivityConcentration;
      model.activityScale = actScale;
      return model;
    }
    if (haveSuv)
    {
      // The SUV-scale factor is defined to produce SUVbw, so it is only
      // meaningful when the input does not claim to be some other SUV type.
      // An absent or empty (0054,1006) is the ordinary case and means BW.
      const auto suvType = ParseSUVTypeProperty(provider);
      if (suvType.has_value() && SUVVariant::BW != suvType.value())
      {
        mitkThrowException(UnsupportedPETUnitsException)
          << "(0054,1001) Units = 'CNTS' with the Philips SUV-scale factor "
             "(7053,xx00) yields SUVbw by definition, but (0054,1006) SUV "
             "Type declares a different variant. Supply the activity "
             "concentration scale factor (7053,xx09) instead, or correct "
             "the SUV Type.";
      }

      // pixel * SUV-scale yields SUVbw directly.
      model.semantics = SUVPixelSemantics::PrenormalizedSUV;
      model.sourceVariant = SUVVariant::BW;
      model.prenormScale = suvScale;
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

mitk::SUVInputModel mitk::ClassifyEnhancedPETInput(const SlicedData *data,
                                                   DICOMReadPolicy /*policy*/)
{
  if (nullptr == data)
  {
    mitkThrow() << "ClassifyEnhancedPETInput: data is null.";
  }

  RequireEnhancedPETFramesResolved(data);

  // ---- The mappings and the transformation, as the reader published them --
  //
  // One property per Real World Value Mapping item and code element, each
  // with a value per slice. The manual allows the code in Code Value, Long
  // Code Value or URN Code Value; whichever is present carries the same
  // meaning.
  std::vector<MappingCode> codes;
  DICOMTagPath unitsCode;
  unitsCode.AddAnySelection(0x0040, 0x9096).AddAnySelection(0x0040, 0x08EA);
  for (const auto element : { 0x0100u, 0x0119u, 0x0120u })
  {
    for (const auto& match :
         GetPropertyByDICOMTagPath(data, DICOMTagPath(unitsCode).AddElement(0x0008, element)))
    {
      if (match.second.IsNotNull())
      {
        codes.push_back({MappingItemIndex(match.first), match.second});
      }
    }
  }

  std::map<int, BaseProperty::ConstPointer> mappingSlopes;
  std::map<int, BaseProperty::ConstPointer> mappingIntercepts;
  DICOMTagPath mapping;
  mapping.AddAnySelection(0x0040, 0x9096);
  for (const auto& match : GetPropertyByDICOMTagPath(data, DICOMTagPath(mapping).AddElement(0x0040, 0x9225)))
  {
    mappingSlopes[MappingItemIndex(match.first)] = match.second;
  }
  for (const auto& match : GetPropertyByDICOMTagPath(data, DICOMTagPath(mapping).AddElement(0x0040, 0x9224)))
  {
    mappingIntercepts[MappingItemIndex(match.first)] = match.second;
  }

  using SUVFunctionalGroupAccess::FirstMatch;
  using SUVFunctionalGroupAccess::MacroAttribute;
  const auto appliedSlope     = FirstMatch(data, MacroAttribute(0x0028, 0x9145, 0x0028, 0x1053));
  const auto appliedIntercept = FirstMatch(data, MacroAttribute(0x0028, 0x9145, 0x0028, 0x1052));
  const auto rescaleType      = FirstMatch(data, MacroAttribute(0x0028, 0x9145, 0x0028, 0x1054));

  // The slots come from the image geometry because the classifier must vouch
  // for exactly the slots the filter scales. A slice that no functional-group
  // property covers (an unexpanded file in a mixed stack, or a per-frame item
  // without either macro) is then refused by the per-slot checks below rather
  // than silently given the series unit.
  std::set<Slot> slots;
  for (TimeStepType t = 0; t < data->GetTimeSteps(); ++t)
  {
    const auto* sliced = data->GetSlicedGeometry(t);
    const unsigned int sliceCount = (nullptr != sliced) ? sliced->GetSlices() : 1u;
    for (unsigned int s = 0; s < sliceCount; ++s)
    {
      slots.insert(Slot(t, static_cast<SlicedData::IndexValueType>(s)));
    }
  }

  // ---- Per slot, then across slots ----------------------------------------
  std::vector<SlotUnit> resolved;
  for (const auto& slot : slots)
  {
    resolved.push_back(ResolveSlotUnit(slot, codes, mappingSlopes, mappingIntercepts,
                                       appliedSlope.GetPointer(), appliedIntercept.GetPointer()));
  }

  const bool anyMapping =
    std::any_of(resolved.cbegin(), resolved.cend(), [](const SlotUnit& s) { return s.anyMapping; });
  if (!anyMapping)
  {
    return ClassifyByRescaleType(data, rescaleType.GetPointer(), slots);
  }

  for (const auto& slotUnit : resolved)
  {
    if (!slotUnit.anyMapping)
    {
      mitkThrowException(MissingDICOMPropertyException)
        << "Enhanced PET: no Real World Value Mapping at " << Describe(slotUnit.slot)
        << ", while other frames carry one.";
    }
    if (slotUnit.unit.has_value())
    {
      continue;
    }
    if (slotUnit.unusableMappings.empty())
    {
      mitkThrowException(UnsupportedPETUnitsException)
        << "Enhanced PET: the Measurement Units Code Sequence at " << Describe(slotUnit.slot)
        << " names only units the SUV pipeline cannot convert: " << Join(slotUnit.unknownCodes)
        << ". Supported: Bq/ml, g/ml{SUVbw}, g/ml{SUVlbm}, g/ml{SUVlbm(Janma)}, "
           "g/ml{SUVlbm(James128)}, g/ml{SUVibw}, cm2/ml{SUVbsa}.";
    }
    mitkThrowException(EnhancedPETMappingNotAppliedException)
      << "Enhanced PET: no Real World Value Mapping describes the pixel values "
         "as loaded at " << Describe(slotUnit.slot) << ". The reader applied the "
         "Pixel Value Transformation " << slotUnit.appliedPair << " (slope/intercept) "
         "and MITK does not apply the mapping, so a mapping is usable only when "
         "its slope and intercept equal that pair. Offered: "
      << Join(slotUnit.unusableMappings) << ".";
  }

  const EnhancedUnit chosen = resolved.front().unit.value();
  for (const auto& slotUnit : resolved)
  {
    if (!(slotUnit.unit.value() == chosen))
    {
      mitkThrowException(EnhancedPETPerFrameVariationException)
        << "The Measurement Units Code Sequence names different units for "
           "different frames. MITK carries one unit per image, so the input "
           "cannot be represented; it is not malformed.";
    }
  }
  return ModelForUnit(chosen);
}
