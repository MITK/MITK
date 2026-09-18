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
#include <string>
#include <vector>
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
  // Values of every property matching a wildcard tag path, in item order.
  // An Enhanced PET per-frame path yields one entry per frame, because each
  // finding keeps its sequence item index in the property name.
  std::vector<std::string> CollectPathValues(const mitk::IPropertyProvider* provider,
                                             const mitk::DICOMTagPath& path)
  {
    std::vector<std::string> values;
    for (const auto& match : mitk::GetPropertyByDICOMTagPath(provider, path))
    {
      if (match.second.IsNotNull())
      {
        values.push_back(match.second->GetValueAsString());
      }
    }
    return values;
  }

  unsigned int ReadNumberOfFrames(const mitk::IPropertyProvider* provider)
  {
    const std::string raw = TrimAndUpper(ReadDICOMString(provider, mitk::DICOMTagPath(0x0028, 0x0008)));
    if (raw.empty())
    {
      return 1u;
    }
    try
    {
      const int frames = std::stoi(raw);
      return (frames > 0) ? static_cast<unsigned int>(frames) : 1u;
    }
    catch (const std::exception&)
    {
      return 1u;
    }
  }

  // A per-frame numeric attribute reduced to the single value the whole
  // volume can be given, or a refusal.
  //
  // Two failure modes, and the second is the subtle one. The values may
  // genuinely differ between frames, which MITK cannot represent. Or the
  // path may resolve to fewer values than there are frames -- an
  // unregistered or mis-specified path yields none at all -- in which case
  // concluding "uniform" would be an accident rather than an observation.
  // Both refuse.
  //
  // Comparison is numeric on purpose: the same slope reads as "4.0" through
  // the Pixel Value Transformation Sequence (a DS) and "4" through the Real
  // World Value Mapping Sequence (an FD), so comparing strings would report
  // variation on a perfectly uniform object.

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
    if ("g/ml{SUVlbm(janma)}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::LBM_Janmahasatian, 1};
    if ("g/ml{SUVlbm(james128)}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::LBM_James128, 1};
    if ("g/ml{SUVibw}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::IBW, 1};
    if ("cm2/ml{SUVbsa}" == code)
      return EnhancedUnit{mitk::SUVPixelSemantics::PrenormalizedSUV, mitk::SUVVariant::BSA, 1};

    return std::nullopt;
  }

  // Outer sequence item index of a resolved property path, i.e. which
  // functional-group item a finding came from. -1 when the path carries no
  // item selection (the shared group read as a plain path).
  int FunctionalGroupItemIndex(const std::string& propertyName)
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

  // The unit each frame resolves to, keyed by functional-group item index.
  // Within one frame several Real World Value Mappings may be offered, and
  // the manual ranks them; across frames they must agree, because MITK
  // cannot carry a per-frame unit.
  std::map<int, EnhancedUnit> CollectPerFrameUnits(const mitk::IPropertyProvider* provider,
                                                   const mitk::DICOMTagPath& groupsRoot)
  {
    std::map<int, EnhancedUnit> byFrame;

    mitk::DICOMTagPath unitsCode(groupsRoot);
    unitsCode.AddAnySelection(0x0040, 0x9096);
    unitsCode.AddAnySelection(0x0040, 0x08EA);

    // The manual allows the code in Code Value, Long Code Value or URN Code
    // Value; whichever is present carries the same meaning.
    for (const auto element : { 0x0100u, 0x0119u, 0x0120u })
    {
      const auto matches =
        mitk::GetPropertyByDICOMTagPath(provider, mitk::DICOMTagPath(unitsCode).AddElement(0x0008, element));
      for (const auto& match : matches)
      {
        if (match.second.IsNull())
        {
          continue;
        }
        const auto unit = MapUcumUnitCode(match.second->GetValueAsString());
        if (!unit.has_value())
        {
          continue;
        }
        const int frame = FunctionalGroupItemIndex(match.first);
        const auto existing = byFrame.find(frame);
        if (existing == byFrame.end() || unit->priority < existing->second.priority)
        {
          byFrame[frame] = unit.value();
        }
      }
    }

    return byFrame;
  }

  std::optional<double> UniformPerFrameNumber(const mitk::IPropertyProvider* provider,
                                              const mitk::DICOMTagPath& path,
                                              const std::string& label,
                                              unsigned int frameCount)
  {
    const auto raw = CollectPathValues(provider, path);
    if (raw.empty())
    {
      return std::nullopt;
    }

    if (frameCount > 1u && raw.size() < frameCount)
    {
      mitkThrowException(mitk::EnhancedPETPerFrameVariationException)
        << "Only " << raw.size() << " value(s) of " << label << " are visible "
           "for an object with " << frameCount << " frames, so the per-frame "
           "values cannot be checked for agreement. Refusing rather than "
           "assuming they agree.";
    }

    double common = 0.0;
    bool haveCommon = false;
    for (const auto& value : raw)
    {
      double parsed = 0.0;
      if (!ParseFiniteDouble(value, parsed))
      {
        continue;
      }
      if (!haveCommon)
      {
        common = parsed;
        haveCommon = true;
      }
      else if (std::fabs(parsed - common) > 1e-9 * std::max(std::fabs(parsed), std::fabs(common)))
      {
        mitkThrowException(mitk::EnhancedPETPerFrameVariationException)
          << label << " differs between frames (" << common << " vs " << parsed
          << "). MITK's DICOM reader models one frame per file, so a "
             "multi-frame object collapses to a single value per attribute "
             "and this one would be applied to the whole volume. The input is "
             "correct; MITK cannot represent it yet.";
      }
    }

    return haveCommon ? std::optional<double>(common) : std::nullopt;
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

mitk::SUVInputModel mitk::ClassifyEnhancedPETInput(const IPropertyProvider *provider,
                                                   DICOMReadPolicy /*policy*/)
{
  if (nullptr == provider)
  {
    mitkThrow() << "ClassifyEnhancedPETInput: provider is null.";
  }

  const unsigned int frameCount = ReadNumberOfFrames(provider);

  DICOMTagPath sharedGroups;
  sharedGroups.AddAnySelection(0x5200, 0x9229);
  DICOMTagPath perFrameGroups;
  perFrameGroups.AddAnySelection(0x5200, 0x9230);

  // ---- The rescale must be the same for every frame ----------------------
  //
  // Not because the pipeline uses it -- GDCM has already applied it to the
  // pixel buffer by the time the filter sees the image -- but because GDCM
  // applies exactly one slope to the whole buffer. Where the frames disagree,
  // the loaded values are wrong for every frame but one, and nothing
  // downstream can tell. Checking here is the only place the disagreement is
  // still visible.
  const auto requireUniform = [&](unsigned int innerGroup, unsigned int innerElement,
                                  unsigned int leafGroup, unsigned int leafElement,
                                  const std::string &label)
  {
    DICOMTagPath perFrame(perFrameGroups);
    perFrame.AddAnySelection(innerGroup, innerElement);
    perFrame.AddElement(leafGroup, leafElement);
    if (UniformPerFrameNumber(provider, perFrame, label, frameCount).has_value())
    {
      return;
    }
    // Absent per frame: a value in the shared group applies to every frame
    // by definition and cannot disagree with itself.
    DICOMTagPath shared(sharedGroups);
    shared.AddAnySelection(innerGroup, innerElement);
    shared.AddElement(leafGroup, leafElement);
    (void)UniformPerFrameNumber(provider, shared, label, 1u);
  };

  requireUniform(0x0028, 0x9145, 0x0028, 0x1053, "(0028,1053) Rescale Slope");
  requireUniform(0x0028, 0x9145, 0x0028, 0x1052, "(0028,1052) Rescale Intercept");
  requireUniform(0x0040, 0x9096, 0x0040, 0x9225, "(0040,9225) Real World Value Slope");
  requireUniform(0x0040, 0x9096, 0x0040, 0x9224, "(0040,9224) Real World Value Intercept");

  SUVInputModel model;

  // ---- Unit, from the Measurement Units Code Sequence --------------------
  auto unitsByFrame = CollectPerFrameUnits(provider, perFrameGroups);
  if (unitsByFrame.empty())
  {
    unitsByFrame = CollectPerFrameUnits(provider, sharedGroups);
  }

  if (!unitsByFrame.empty())
  {
    const EnhancedUnit chosen = unitsByFrame.begin()->second;
    for (const auto &entry : unitsByFrame)
    {
      if (!(entry.second == chosen))
      {
        mitkThrowException(EnhancedPETPerFrameVariationException)
          << "The Measurement Units Code Sequence names different units for "
             "different frames. MITK carries one unit per image, so the input "
             "cannot be represented; it is not malformed.";
      }
    }

    model.semantics = chosen.semantics;
    if (SUVPixelSemantics::PrenormalizedSUV == chosen.semantics)
    {
      model.sourceVariant = chosen.variant;
      model.prenormScale = 1.0;
    }
    else
    {
      model.activityScale = 1.0;
    }
    return model;
  }

  // ---- Fallback: Rescale Type from the Pixel Value Transformation --------
  //
  // The manual marks this a fallback because Rescale Type is supposed to be
  // "US" for PET, so a unit hiding there is a departure from the standard
  // rather than the intended place to look.
  DICOMTagPath rescaleType(perFrameGroups);
  rescaleType.AddAnySelection(0x0028, 0x9145).AddElement(0x0028, 0x1054);
  std::string rawRescaleType = ReadDICOMString(provider, rescaleType);
  if (rawRescaleType.empty())
  {
    DICOMTagPath sharedRescaleType(sharedGroups);
    sharedRescaleType.AddAnySelection(0x0028, 0x9145).AddElement(0x0028, 0x1054);
    rawRescaleType = ReadDICOMString(provider, sharedRescaleType);
  }

  const std::string units = TrimAndUpper(rawRescaleType);
  if ("BQML" == units)
  {
    model.semantics = SUVPixelSemantics::ActivityConcentration;
    model.activityScale = 1.0;
    return model;
  }
  if ("GML" == units)
  {
    const auto suvType = ParseSUVTypeProperty(provider);
    model.semantics = SUVPixelSemantics::PrenormalizedSUV;
    model.sourceVariant = suvType.value_or(SUVVariant::BW);
    model.prenormScale = 1.0;
    return model;
  }
  if ("CM2ML" == units)
  {
    model.semantics = SUVPixelSemantics::PrenormalizedSUV;
    model.sourceVariant = SUVVariant::BSA;
    model.prenormScale = 1.0;
    return model;
  }

  if (!units.empty())
  {
    mitkThrowException(UnsupportedPETUnitsException)
      << "Enhanced PET: no usable Measurement Units Code Sequence, and "
         "(0028,1054) Rescale Type = '" << rawRescaleType << "' is not one of "
         "BQML, GML, CM2ML.";
  }

  mitkThrowException(MissingDICOMPropertyException)
    << "Enhanced PET: the pixel unit could not be determined. Neither the "
       "Measurement Units Code Sequence (0040,08EA) inside the Real World "
       "Value Mapping Sequence nor (0028,1054) Rescale Type yielded a unit "
       "the SUV pipeline converts.";
}
