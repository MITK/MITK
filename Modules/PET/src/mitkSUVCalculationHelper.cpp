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

#include <mitkSUVCalculationHelper.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>

#include <dcmtk/dcmdata/dcvrdt.h>

// Mitk
#include <mitkBaseProperty.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMTimeUtil.h>
#include <mitkDICOMTagPath.h>
#include <mitkIPropertyProvider.h>
#include <mitkLog.h>
#include <mitkSlicedGeometry3D.h>

#include <chrono>
namespace
{
  // String helpers used for the (0054,1102) value comparison.
  std::string TrimAsciiWhitespace(const std::string& s)
  {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
      return {};
    }
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
  }

  std::string ToUpperAscii(std::string s)
  {
    std::transform(s.begin(), s.end(), s.begin(),
      [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
  }

  // Pull the first matching property as a DICOMProperty so the caller can
  // index by (timestep, slice). Returns nullptr if no match. Wraps
  // mitk::GetPropertyByDICOMTagPath so per-slice access stays available
  // alongside the simpler mitk::GetFirstDICOMValueAsString convenience.
  const mitk::DICOMProperty* FindFirstDICOMProperty(
    const mitk::IPropertyProvider* provider,
    const mitk::DICOMTagPath& path)
  {
    const auto matches = mitk::GetPropertyByDICOMTagPath(provider, path);
    if (matches.empty())
    {
      return nullptr;
    }
    if (matches.size() > 1)
    {
      MITK_WARN << "Multiple properties match DICOM tag path " << path.ToStr()
                << "; using the first match: " << matches.begin()->first;
    }
    return dynamic_cast<const mitk::DICOMProperty*>(matches.begin()->second.GetPointer());
  }

  // Splice an optional date string and a time string into an OFDateTime.
  // Returns true on success.
  bool ConvertDICOMDateTimeString(const std::string& dateString,
                                  const std::string& timeString,
                                  OFDateTime& time)
  {
    OFString content(timeString.c_str());

    if (!dateString.empty())
    {
      content = OFString(dateString.c_str()).append(content);
    }

    const OFCondition result = DcmDateTime::getOFDateTimeFromString(content, time);

    return result.good();
  }

  // Returns (reference - injection) in seconds (the shared util reports ms).
  double DurationInSeconds(const OFDateTime& injection, const OFDateTime& reference)
  {
    return mitk::ComputeMiliSecDuration(injection, reference) / 1000.0;
  }

  // Seconds since midnight of the instant \a time denotes, on the same
  // normalization ConvertOFDateTimeToTimePoint applies: a stamp carrying a
  // UTC offset is shifted by it, a stamp without one is taken as local to
  // itself. Both halves of the administration-time rule below must agree on
  // this or they measure different things -- and no benchmark DRO mixes the
  // two forms, so nothing but this comment and its unit test guards it.
  double SecondsOfDayUTC(const OFDateTime& time)
  {
    constexpr long long kMsPerDay = 24LL * 60LL * 60LL * 1000LL;
    const auto ms = mitk::ConvertOFDateTimeToTimePoint(time).time_since_epoch().count();
    // Floor-modulo, not truncating: anonymized inputs carry pre-1970 dates
    // (DRO_4_4 uses 1960-01-01), where a truncating % yields a negative
    // time of day.
    long long remainder = ms % kMsPerDay;
    if (remainder < 0)
    {
      remainder += kMsPerDay;
    }
    return static_cast<double>(remainder) / 1000.0;
  }

  double NormalizeSecondsOfDay(double seconds)
  {
    constexpr double kSecondsPerDay = 24.0 * 60.0 * 60.0;
    double normalized = std::fmod(seconds, kSecondsPerDay);
    if (normalized < 0.0)
    {
      normalized += kSecondsPerDay;
    }
    return normalized;
  }

  /** The two radiopharmaceutical administration-time tags, read once.
   *
   * Both are image-level, so they are resolved outside the per-slice loop.
   * Either may be absent -- that combination is a refusal condition the
   * consumer reports, not something this function pre-validates. A tag that
   * is present but unparseable is an error rather than an absence: falling
   * through to the other tag would silently compute from a different
   * instant than the one the input names. */
  struct AdministrationTimeTags
  {
    bool       haveStartDateTime = false;   // (0018,1078) present and parsed
    OFDateTime startDateTime;
    bool       haveStartTime = false;       // (0018,1072) present and parsed
    double     startTimeOfDaySeconds = 0.0; // valid only if haveStartTime
  };

  AdministrationTimeTags ResolveAdministrationTimeTags(const mitk::IPropertyProvider* provider)
  {
    AdministrationTimeTags tags;

    mitk::DICOMTagPath startDateTimePath;
    startDateTimePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1078);
    if (const auto* prop = FindFirstDICOMProperty(provider, startDateTimePath))
    {
      const std::string raw = prop->GetValue(0, 0, true, true);
      // (0018,1078) is a DT VR -- date and time are already encoded together.
      if (!ConvertDICOMDateTimeString("", raw, tags.startDateTime))
      {
        mitkThrowException(mitk::InvalidDICOMPropertyValueException)
          << "Cannot parse Radiopharmaceutical Start DateTime (0018,1078) value '"
          << raw << "'.";
      }
      tags.haveStartDateTime = true;
    }

    mitk::DICOMTagPath startTimePath;
    startTimePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1072);
    if (const auto* prop = FindFirstDICOMProperty(provider, startTimePath))
    {
      const std::string raw = prop->GetValue(0, 0, true, true);
      OFDateTime parsed;
      // (0018,1072) is a TM VR and carries no date. Any date parses it into
      // an absolute instant; only its time of day is ever read back.
      if (!ConvertDICOMDateTimeString("19700101", raw, parsed))
      {
        mitkThrowException(mitk::InvalidDICOMPropertyValueException)
          << "Cannot parse Radiopharmaceutical Start Time (0018,1072) value '"
          << raw << "'.";
      }
      tags.startTimeOfDaySeconds = SecondsOfDayUTC(parsed);
      tags.haveStartTime = true;
    }

    return tags;
  }

  // Above this half-life an administration date that is wrong by a whole
  // day still produces a plausible SUV, so the substitution below is not
  // permitted and the input has to be refused outright.
  constexpr double kDateSubstitutionHalfLifeLimitSeconds = 41400.0;

  // Reconstruct the decay duration when the stored administration date
  // cannot be trusted: keep the stored time of day, take the date from the
  // reference instant. Because the two then share a date by construction,
  // "reference minus administration" collapses to plain time-of-day
  // arithmetic -- and subtracting a day from the administration datetime is
  // the same operation as adding one to the difference. That equivalence is
  // what lets this avoid calendar arithmetic entirely, and with it a whole
  // class of month- and year-boundary bugs.
  double SubstituteAdministrationDate(double referenceTimeOfDaySeconds,
                                      double administrationTimeOfDaySeconds,
                                      std::vector<mitk::SUVAdaptation>& adaptations)
  {
    constexpr double kSecondsPerDay = 24.0 * 60.0 * 60.0;

    double duration = referenceTimeOfDaySeconds - administrationTimeOfDaySeconds;
    if (duration < mitk::kEarliestDecayDurationSeconds)
    {
      duration += kSecondsPerDay;
      MITK_WARN << "Reconstructed administration datetime falls after the "
                   "decay-correction reference time; moving it back one day "
                   "per the IBSI-SUV recommendation. Decay duration: "
                << duration << " s.";
      adaptations.push_back({mitk::SUVAdaptationRule::AdministrationTimeShiftedBackOneDay,
                             "", "", std::to_string(duration)});
    }
    return duration;
  }

  /** Residual decay duration for one (timestep, slice), per IBSI-SUV v3.0.1.
   *
   * The reference instant arrives as a stored datetime plus an offset in
   * seconds rather than as a single OFDateTime, because two of the four
   * call sites compute it as "acquisition datetime + a correction" and
   * DCMTK offers no datetime-plus-duration arithmetic. */
  double ResolveDecayDurationSeconds(const AdministrationTimeTags& admin,
                                     const OFDateTime& referenceBase,
                                     double referenceOffsetSeconds,
                                     double halfLifeSeconds,
                                     mitk::DICOMReadPolicy policy,
                                     std::vector<mitk::SUVAdaptation>& adaptations)
  {
    if (!std::isfinite(halfLifeSeconds) || halfLifeSeconds <= 0.0)
    {
      // Every branch below is keyed on the half-life -- even the one that
      // uses (0018,1078) verbatim, whose acceptance window is 2 * T_half.
      // Without it there is no rule to apply, so refuse rather than invent
      // a window.
      mitkThrowException(mitk::MissingDICOMPropertyException)
        << "Resolving the radiopharmaceutical administration time requires a "
           "positive radionuclide half-life (got " << halfLifeSeconds
        << " s). Provide it via an explicit override or supply DICOM "
           "(0018,1075).";
    }

    const double referenceTimeOfDay =
      NormalizeSecondsOfDay(SecondsOfDayUTC(referenceBase) + referenceOffsetSeconds);

    if (admin.haveStartDateTime)
    {
      const double offset =
        DurationInSeconds(admin.startDateTime, referenceBase) + referenceOffsetSeconds;

      if (offset >= mitk::kEarliestDecayDurationSeconds && offset < 2.0 * halfLifeSeconds)
      {
        return offset;
      }

      if (halfLifeSeconds >= kDateSubstitutionHalfLifeLimitSeconds)
      {
        mitkThrowException(mitk::UnrecoverableAdministrationDateException)
          << "(0018,1078) Radiopharmaceutical Start DateTime yields a decay "
             "duration of " << offset << " s, outside the plausible window "
             "[" << mitk::kEarliestDecayDurationSeconds << " s, 2 * half-life). "
             "Reconstructing the administration date from the reference "
             "datetime is only permitted below a half-life of "
          << kDateSubstitutionHalfLifeLimitSeconds << " s, and this "
             "radionuclide's is " << halfLifeSeconds
          << " s. Re-export the data with a correct administration date.";
      }

      if (mitk::DICOMReadPolicy::Strict == policy)
      {
        mitkThrowException(mitk::AdministrationDateSubstitutionRefusedException)
          << "(0018,1078) Radiopharmaceutical Start DateTime yields an "
             "implausible decay duration of " << offset << " s. The IBSI-SUV "
             "recommendation reconstructs the administration date from the "
             "decay-correction reference datetime, but "
             "DICOMReadPolicy::Strict is active. Re-export the data with a "
             "correct administration date or relax the policy.";
      }

      MITK_WARN << "(0018,1078) Radiopharmaceutical Start DateTime yields an "
                   "implausible decay duration of " << offset
                << " s; keeping its time of day and taking the date from the "
                   "decay-correction reference datetime per the IBSI-SUV "
                   "recommendation.";
      adaptations.push_back({mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartDateTime,
                             "(0018,1078)", std::to_string(offset), ""});

      return SubstituteAdministrationDate(referenceTimeOfDay,
                                          SecondsOfDayUTC(admin.startDateTime),
                                          adaptations);
    }

    if (admin.haveStartTime)
    {
      if (halfLifeSeconds >= kDateSubstitutionHalfLifeLimitSeconds)
      {
        mitkThrowException(mitk::UnrecoverableAdministrationDateException)
          << "Only (0018,1072) Radiopharmaceutical Start Time is available, "
             "which carries no date. Reconstructing one from the "
             "decay-correction reference datetime is only permitted below a "
             "half-life of " << kDateSubstitutionHalfLifeLimitSeconds
          << " s, and this radionuclide's is " << halfLifeSeconds
          << " s -- an uptake longer than a day would be indistinguishable "
             "from a short one. Re-export the data with (0018,1078) "
             "Radiopharmaceutical Start DateTime.";
      }

      if (mitk::DICOMReadPolicy::Strict == policy)
      {
        mitkThrowException(mitk::AdministrationDateSubstitutionRefusedException)
          << "Only (0018,1072) Radiopharmaceutical Start Time is available. "
             "The IBSI-SUV recommendation reconstructs the administration "
             "date from the decay-correction reference datetime, but "
             "DICOMReadPolicy::Strict is active. Re-export the data with "
             "(0018,1078) Radiopharmaceutical Start DateTime or relax the "
             "policy.";
      }

      MITK_WARN << "Only (0018,1072) Radiopharmaceutical Start Time is "
                   "available; taking the administration date from the "
                   "decay-correction reference datetime per the IBSI-SUV "
                   "recommendation.";
      adaptations.push_back({mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartTime,
                             "(0018,1072)", "", ""});

      return SubstituteAdministrationDate(referenceTimeOfDay,
                                          admin.startTimeOfDaySeconds,
                                          adaptations);
    }

    mitkThrowException(mitk::MissingDICOMPropertyException)
      << "No radiopharmaceutical injection time available: neither "
         "(0018,1078) Radiopharmaceutical Start DateTime nor (0018,1072) "
         "Radiopharmaceutical Start Time was found.";
  }

  // Pull a named property (attached out-of-band — e.g. the lifted vendor
  // private datetimes from BaseDICOMReaderService, see issue #783) as a
  // DICOMProperty so the caller can read per-(t, s) values uniformly with
  // the standard tag-of-interest properties.
  const mitk::DICOMProperty* FindNamedDICOMProperty(const mitk::IPropertyProvider* provider,
                                                    const std::string& propertyName)
  {
    if (nullptr == provider) return nullptr;
    auto baseProp = provider->GetConstProperty(propertyName.c_str());
    if (baseProp.IsNull()) return nullptr;
    return dynamic_cast<const mitk::DICOMProperty*>(baseProp.GetPointer());
  }

  // Parse a DICOM DT-format string ("YYYYMMDDHHMMSS[.FFFFFF][&ZZXX]") into
  // an OFDateTime. Returns true on success.
  bool ParseDICOMDateTime(const std::string& dt, OFDateTime& out)
  {
    return DcmDateTime::getOFDateTimeFromString(OFString(dt.c_str()), out).good();
  }

  // Read a numeric DICOM tag at (timestep, slice). Returns NaN if the tag
  // is absent at that slot or cannot be parsed.
  double ReadNumericTagAt(const mitk::DICOMProperty* prop,
                          mitk::TimeStepType t,
                          mitk::SlicedData::IndexValueType s)
  {
    if (nullptr == prop) return std::numeric_limits<double>::quiet_NaN();
    const std::string raw = prop->GetValue(t, s, true, true);
    if (raw.empty()) return std::numeric_limits<double>::quiet_NaN();
    return mitk::ConvertDICOMStrToValue<double>(raw);
  }

  // Compare two OFDateTimes at second resolution: true if they represent
  // the same wall-clock second (sub-second jitter is ignored). Used by
  // DC=START Step 2 (AcquisitionTime equals SeriesTime in seconds).
  bool EqualAtSecondResolution(const OFDateTime& a, const OFDateTime& b)
  {
    return std::fabs(DurationInSeconds(a, b)) < 1.0;
  }

  // Closed-form average count-rate time per IBSI-SUV recommendation
  // (suv_text.txt:820-826):
  //
  //     T_ave = (1/lambda) * ln((lambda * T) / (1 - exp(-lambda * T)))
  //
  // with lambda = ln(2) / T_half and T = ActualFrameDuration.
  //
  // Physically: a non-decay-corrected voxel value is the count rate
  // averaged over the frame; T_ave is the time inside the frame at
  // which the *instantaneous* count rate equals that average. As
  // lambda*T -> 0 the result approaches T/2 (frame midpoint). For 18F
  // (lambda ~ 1.05e-4 /s) and a 5-minute frame, lambda*T ~ 0.03, so
  // the formula and T/2 agree to about one part in a thousand.
  //
  // We use std::expm1(-lambda*T) instead of (1 - exp(-lambda*T)) for
  // numerical stability when lambda*T is small.
  double ComputeTAveSeconds(double frameDurationSeconds, double halfLifeSeconds)
  {
    if (frameDurationSeconds <= 0.0 || halfLifeSeconds <= 0.0
        || !std::isfinite(frameDurationSeconds) || !std::isfinite(halfLifeSeconds))
    {
      mitkThrowException(mitk::InvalidDICOMPropertyValueException)
        << "ComputeTAveSeconds requires positive, finite frame duration and "
           "half-life (got T=" << frameDurationSeconds
        << " s, T_half=" << halfLifeSeconds << " s).";
    }
    const double lambda = std::log(2.0) / halfLifeSeconds;
    const double lambdaT = lambda * frameDurationSeconds;
    // (1 - exp(-lambdaT)) computed stably as -expm1(-lambdaT).
    const double denom = -std::expm1(-lambdaT);
    return (1.0 / lambda) * std::log(lambdaT / denom);
  }
}


namespace
{
  // Extract the outer (i.e. first) SequenceSelection index from a DICOMTagPath.
  // For a path like (0054,0016).[N].(0018,1075) returns N. For nested
  // sequences like (0054,0016).[N].(0054,0300).[M].(0008,0104) it still
  // returns the outermost index N (the (0054,0016) item index), which is the
  // identity used to pair half-life / dose / name.
  // Returns -1 if the path has no SequenceSelection node.
  mitk::DICOMTagPath::ItemSelectionIndex
  OuterSequenceIndex(const mitk::DICOMTagPath& path)
  {
    for (const auto& node : path.GetNodes())
    {
      if (node.type == mitk::DICOMTagPath::NodeInfo::NodeType::SequenceSelection)
      {
        return node.selection;
      }
    }
    return -1;
  }
}

std::vector<mitk::RadiopharmaceuticalInfo>
mitk::GetRadiopharmaceuticalInfos(const mitk::IPropertyProvider* provider,
                                  mitk::DICOMReadPolicy policy,
                                  std::vector<mitk::SUVAdaptation>& adaptations)
{
  using IndexedMap = std::map<DICOMTagPath::ItemSelectionIndex, RadiopharmaceuticalInfo>;
  IndexedMap byIndex;

  // Helper: enumerate properties matching a query path, group the values by
  // the outer-sequence (RPI) item index, and apply the assigner to the
  // per-item RadiopharmaceuticalInfo.
  auto enumerate = [&](const DICOMTagPath& queryPath,
                       const std::function<void(RadiopharmaceuticalInfo&,
                                                const std::string&)>& assigner)
  {
    const auto matches = mitk::GetPropertyByDICOMTagPath(provider, queryPath);
    for (const auto& finding : matches)
    {
      const DICOMTagPath storedPath = PropertyNameToDICOMTagPath(finding.first);
      const auto idx = OuterSequenceIndex(storedPath);
      if (idx < 0)
      {
        continue;
      }
      assigner(byIndex[idx], finding.second->GetValueAsString());
    }
  };

  // Half-life (0018,1075).
  DICOMTagPath halfLifePath;
  halfLifePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1075);
  enumerate(halfLifePath, [](RadiopharmaceuticalInfo& info, const std::string& v)
  {
    info.halfLifeSeconds = ConvertDICOMStrToValue<double>(v);
  });

  // Total dose (0018,1074). The DICOM standard prescribes Bq, but some
  // scanners and post-processing pipelines store the value in MBq.
  // Empirically the two regimes are well separated: clinical FDG doses
  // cluster around 4e2 (MBq) and 4e8 (Bq), with no plausible value in
  // between. Per the IBSI-SUV recommendation, values strictly below the
  // 1e4 threshold are interpreted as MBq and converted to Bq.
  // DICOMReadPolicy controls the response: Lenient applies the
  // conversion with a WARN; Strict refuses it and raises a dedicated
  // exception so callers can surface the input issue.
  DICOMTagPath dosePath;
  dosePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1074);
  enumerate(dosePath, [policy, &adaptations](RadiopharmaceuticalInfo& info, const std::string& v)
  {
    const double raw = ConvertDICOMStrToValue<double>(v);
    if (raw > 0.0 && raw < 1.0e4)
    {
      if (policy == DICOMReadPolicy::Strict)
      {
        mitkThrowException(ImplausibleRadionuclideDoseException)
          << "Radionuclide Total Dose (0018,1074) value " << raw
          << " is below the 1e4 plausibility threshold and would be "
             "reinterpreted as MBq under the IBSI-SUV recommendation, "
             "but DICOMReadPolicy::Strict is active. Re-export the "
             "input with a Bq-magnitude value or rerun in lenient mode.";
      }
      const double converted = raw * 1.0e6;
      MITK_WARN << "Radionuclide Total Dose (0018,1074) value " << raw
                << " is below the 1e4 plausibility threshold; "
                   "interpreting as MBq and converting to Bq (= "
                << converted << " Bq) per IBSI-SUV recommendation.";
      adaptations.push_back({SUVAdaptationRule::DoseReinterpretedAsMBq,
                             "(0018,1074)", v, std::to_string(converted)});
      info.totalDoseBq = converted;
    }
    else
    {
      info.totalDoseBq = raw;
    }
  });

  // Radionuclide code meaning, nested in (0054,0300). The outer index we
  // care about is the (0054,0016) item index; multiple inner radionuclide
  // entries within one outer item are concatenated with spaces (matches the
  // historical GetRadionuclideNames behaviour, scoped per outer item).
  DICOMTagPath namePath;
  namePath.AddAnySelection(0x0054, 0x0016).AddAnySelection(0x0054, 0x0300).AddElement(0x0008, 0x0104);
  enumerate(namePath, [](RadiopharmaceuticalInfo& info, const std::string& v)
  {
    std::string cleaned = v;
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '^'), cleaned.end());
    if (!info.name.empty())
    {
      info.name += " ";
    }
    info.name += cleaned;
  });

  // std::map iterates in key order, so the resulting vector is ordered by
  // the outer-sequence item index.
  std::vector<RadiopharmaceuticalInfo> result;
  result.reserve(byIndex.size());
  for (const auto& entry : byIndex)
  {
    result.push_back(entry.second);
  }
  return result;
}

double mitk::GetPatientsWeight(const mitk::IPropertyProvider* provider)
{
  if (nullptr == provider)
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "Cannot retrieve patient weight: property provider is null.";
  }

  const DICOMTagPath weightPath(0x0010, 0x1030);
  const auto props = mitk::GetPropertyByDICOMTagPath(provider, weightPath);

  if (props.empty())
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "Cannot retrieve patient weight: no DICOM property for tag "
         "(0010,1030) Patient Weight was found.";
  }

  return ConvertDICOMStrToValue<double>(props.begin()->second->GetValueAsString());
}

double mitk::GetPatientsHeight(const mitk::IPropertyProvider* provider)
{
  if (nullptr == provider)
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "Cannot retrieve patient height: property provider is null.";
  }

  const DICOMTagPath heightPath(0x0010, 0x1020);
  const auto props = mitk::GetPropertyByDICOMTagPath(provider, heightPath);

  if (props.empty())
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "Cannot retrieve patient height: no DICOM property for tag "
         "(0010,1020) Patient Size was found.";
  }

  return ConvertDICOMStrToValue<double>(props.begin()->second->GetValueAsString());
}

mitk::Sex mitk::GetPatientsSex(const mitk::IPropertyProvider* provider)
{
  if (nullptr == provider)
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "Cannot retrieve patient sex: property provider is null.";
  }

  const DICOMTagPath sexPath(0x0010, 0x0040);
  const std::string raw = mitk::GetFirstDICOMValueAsString(provider, sexPath);
  if (raw.empty())
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "Cannot retrieve patient sex: no DICOM property for tag "
         "(0010,0040) Patient Sex was found.";
  }

  const std::string normalized = ToUpperAscii(TrimAsciiWhitespace(raw));

  if ("M" == normalized) return Sex::Male;
  if ("F" == normalized) return Sex::Female;
  if ("O" == normalized) return Sex::Other;

  mitkThrowException(InvalidDICOMPropertyValueException)
    << "DICOM tag (0010,0040) Patient Sex holds unsupported value '"
    << raw << "'. Expected one of M, F, O.";
}

mitk::ManufacturerFamily mitk::GetManufacturerFamily(const mitk::IPropertyProvider* provider)
{
  if (nullptr == provider)
  {
    return ManufacturerFamily::Other;
  }

  const DICOMTagPath manufPath(0x0008, 0x0070);
  const std::string raw = mitk::GetFirstDICOMValueAsString(provider, manufPath);
  const std::string normalized = ToUpperAscii(TrimAsciiWhitespace(raw));

  // Substring match: real-world values include "SIEMENS Healthineers",
  // "GE MEDICAL SYSTEMS", "GE HEALTHCARE", "Philips Medical Systems".
  // GE matches require word-boundary handling so a hypothetical
  // manufacturer string starting with "GE" but not denoting GE
  // Healthcare (none observed in practice) would fall through to Other.
  if (std::string::npos != normalized.find("SIEMENS"))
  {
    return ManufacturerFamily::Siemens;
  }
  if (std::string::npos != normalized.find("PHILIPS"))
  {
    return ManufacturerFamily::Philips;
  }
  if ("GE" == normalized
      || (normalized.size() >= 3 && normalized.substr(0, 3) == "GE "))
  {
    return ManufacturerFamily::GE;
  }
  return ManufacturerFamily::Other;
}

mitk::DecayCorrectionStrategy mitk::GetDecayCorrectionStrategy(const mitk::IPropertyProvider* provider)
{
  const DICOMTagPath decayCorrPath(0x0054, 0x1102);

  const std::string raw = mitk::GetFirstDICOMValueAsString(provider, decayCorrPath);
  if (raw.empty())
  {
    mitkThrowException(MissingDICOMPropertyException)
      << "Cannot determine decay correction strategy: DICOM tag "
         "(0054,1102) Decay Correction is missing.";
  }

  const std::string normalized = ToUpperAscii(TrimAsciiWhitespace(raw));

  if ("ADMIN" == normalized) return DecayCorrectionStrategy::Admin;
  if ("START" == normalized) return DecayCorrectionStrategy::Start;
  if ("NONE"  == normalized) return DecayCorrectionStrategy::None;

  mitkThrowException(InvalidDICOMPropertyValueException)
    << "DICOM tag (0054,1102) Decay Correction holds unsupported value '"
    << raw << "'. Expected one of ADMIN, START, NONE.";
}

namespace
{
  // For ADMIN and START, every (timestep, slice) entry holds the same value.
  // The iteration source is purely the SlicedData time geometry; no DICOM
  // acquisition tag is required for the iteration.
  void FillUniformDecayMap(const mitk::SlicedData* data,
                           double value,
                           mitk::DecayTimeMapType& outMap)
  {
    const auto timeSteps = data->GetTimeSteps();
    for (mitk::TimeStepType t = 0; t < timeSteps; ++t)
    {
      const auto* sliced = data->GetSlicedGeometry(t);
      const unsigned int slices = (nullptr != sliced) ? sliced->GetSlices() : 1u;
      auto& sliceMap = outMap[t];
      for (unsigned int s = 0; s < slices; ++s)
      {
        sliceMap[static_cast<mitk::SlicedData::IndexValueType>(s)] = value;
      }
    }
  }

  // Read a date string we can use for assembling a SeriesTime / fallback
  // injection-time date. Prefers SeriesDate (0008,0021), falls back to the
  // AcquisitionDate at slot (0,0). Returns empty if neither is available.
  std::string ResolveSeriesReferenceDate(const mitk::IPropertyProvider* provider)
  {
    const std::string seriesDate =
      mitk::GetFirstDICOMValueAsString(provider, mitk::DICOMTagPath(0x0008, 0x0021));
    if (!seriesDate.empty())
    {
      return seriesDate;
    }
    return mitk::GetFirstDICOMValueAsString(provider, mitk::DICOMTagPath(0x0008, 0x0022));
  }

  // Resolve the half-life: use the caller-supplied value if finite,
  // otherwise fall back to item 0 of the Radiopharmaceutical Information
  // Sequence. Returns NaN if neither source yields a half-life. Multi-tracer
  // callers should always supply the resolved half-life explicitly so the
  // selection matches their tracer-index choice.
  double ResolveHalfLifeSeconds(const mitk::IPropertyProvider* provider,
                                double providedHalfLifeSeconds)
  {
    if (std::isfinite(providedHalfLifeSeconds) && providedHalfLifeSeconds > 0.0)
    {
      return providedHalfLifeSeconds;
    }
    // Only the half-life is wanted here, so the dose-plausibility policy is
    // irrelevant and the resulting record is discarded: the filter reads the
    // sequence itself and is the one call that owns the dose adaptation.
    // Reading leniently also keeps this lookup from throwing on an
    // implausible dose that the caller may never consult.
    std::vector<mitk::SUVAdaptation> ignoredAdaptations;
    const auto infos = mitk::GetRadiopharmaceuticalInfos(
      provider, mitk::DICOMReadPolicy::Lenient, ignoredAdaptations);
    if (infos.empty())
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return infos[0].halfLifeSeconds;
  }
}

// "Step 1" .. "Step 4" in the comments below refer to the canonical
// DC=START fallback chain documented in the public Doxygen of
// DeduceDecayCorrection() in mitkSUVCalculationHelper.h (Doxygen
// anchor: DCStartFallbackChain). Tests use the same numbering.
mitk::DecayCorrectionInfo mitk::DeduceDecayCorrection(const mitk::SlicedData* data,
                                                      double halfLifeSeconds,
                                                      DICOMReadPolicy policy)
{
  if (nullptr == data)
  {
    mitkThrow() << "DeduceDecayCorrection: input data is null.";
  }

  DecayCorrectionInfo info;
  info.strategy = GetDecayCorrectionStrategy(data);

  switch (info.strategy)
  {
    case DecayCorrectionStrategy::Admin:
    {
      FillUniformDecayMap(data, 0.0, info.decayTimes);
      return info;
    }

    case DecayCorrectionStrategy::Start:
    {
      // Common: parse SeriesDate/Time and injection time. Required for
      // every fallback step.
      const std::string referenceDate = ResolveSeriesReferenceDate(data);
      if (referenceDate.empty())
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Strategy START requires a series / acquisition date "
             "(0008,0021 or 0008,0022) to assemble an absolute Series Time. "
             "Neither tag was found.";
      }

      const std::string seriesTime =
        mitk::GetFirstDICOMValueAsString(data, DICOMTagPath(0x0008, 0x0031));
      if (seriesTime.empty())
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Strategy START requires (0008,0031) Series Time. Tag is missing.";
      }

      OFDateTime ofSeriesTime;
      if (!ConvertDICOMDateTimeString(referenceDate, seriesTime, ofSeriesTime))
      {
        mitkThrowException(InvalidDICOMPropertyValueException)
          << "Cannot parse Series Date+Time '" << referenceDate << seriesTime << "'.";
      }

      const auto admin = ResolveAdministrationTimeTags(data);
      const auto manuf = GetManufacturerFamily(data);
      // Hoisted above Step 1: every reference-time rule now feeds
      // ResolveDecayDurationSeconds, whose acceptance window is keyed on the
      // half-life, so it is needed before the first candidate is evaluated
      // rather than only by the T_ave formula further down.
      const double halfLife = ResolveHalfLifeSeconds(data, halfLifeSeconds);

      // ---- Step 1: vendor private datetime (per slice via lifted property) ----
      // The PET reader (BaseDICOMReaderService) lifts these values out of
      // the corresponding private creator blocks per file and stores them
      // as a TemporoSpatialStringProperty keyed by (timestep, slice). See
      // issue #783 for the design discussion. We read per-(t, s) so that
      // multi-bed acquisitions whose private datetime varies per file are
      // handled correctly; uniform-across-files data degenerates to the
      // same value for every slot via the property's default-context
      // fallback, yielding a uniform decay map without special-casing.
      if (ManufacturerFamily::Siemens == manuf || ManufacturerFamily::GE == manuf)
      {
        const auto* privateProp = FindNamedDICOMProperty(data,
          (ManufacturerFamily::Siemens == manuf)
            ? "mitk.pet.SiemensDecayDateTime"
            : "mitk.pet.GEScanDateTime");
        if (nullptr != privateProp)
        {
          const auto timeSteps = data->GetTimeSteps();
          DecayTimeMapType candidateMap;
          bool allSlicesValid = true;

          for (TimeStepType t = 0; t < timeSteps && allSlicesValid; ++t)
          {
            const auto* sliced = data->GetSlicedGeometry(t);
            const unsigned int slices = (nullptr != sliced) ? sliced->GetSlices() : 1u;
            auto& sliceMap = candidateMap[t];

            for (unsigned int s = 0; s < slices && allSlicesValid; ++s)
            {
              const std::string privateDt = privateProp->GetValue(t, s, true, true);
              OFDateTime ofPrivate;
              if (privateDt.empty() || !ParseDICOMDateTime(privateDt, ofPrivate))
              {
                allSlicesValid = false;
                break;
              }
              // The vendor private datetime carries no plausibility
              // precondition in the recommendation: "the dose should be
              // corrected to the datetime stored in the private scan start
              // datetime if it is present". Once this rule applies we commit
              // to it; tolerance for a slightly negative offset now lives in
              // the administration-time window, where it belongs.
              sliceMap[static_cast<SlicedData::IndexValueType>(s)] =
                ResolveDecayDurationSeconds(admin, ofPrivate, 0.0, halfLife,
                                            policy, info.adaptations);
            }
          }

          if (allSlicesValid)
          {
            info.decayTimes = std::move(candidateMap);
            return info;
          }
          // Any per-slice precondition failure -> fall through to Step 2.
        }
      }

      // ---- Per-slice acquisition-time tags (Steps 2/3/4) ----
      const auto* acqDateProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0008, 0x0022));
      const auto* acqTimeProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0008, 0x0032));
      const bool haveAcqTags = (nullptr != acqDateProp) && (nullptr != acqTimeProp);

      // ---- Step 2: AcquisitionTime equals SeriesTime in seconds ----
      // Spec preconditions: per-slice AcqTime non-negative, AcqTime equals
      // SeriesTime in seconds. There is no manufacturer condition: an
      // AcquisitionTime that already equals the SeriesTime identifies the
      // reference instant directly, whoever built the scanner. The
      // condition is evaluated at slice 0 (single-bed scans, or first bed
      // of multi-bed scans, per the spec).
      if (haveAcqTags)
      {
        const std::string firstAcqDate = acqDateProp->GetValue(0, 0, true, true);
        const std::string firstAcqTime = acqTimeProp->GetValue(0, 0, true, true);
        OFDateTime firstAcq;
        if (ConvertDICOMDateTimeString(firstAcqDate, firstAcqTime, firstAcq)
            && EqualAtSecondResolution(firstAcq, ofSeriesTime))
        {
          const auto timeSteps = data->GetTimeSteps();
          for (TimeStepType t = 0; t < timeSteps; ++t)
          {
            const auto* sliced = data->GetSlicedGeometry(t);
            const unsigned int slices = (nullptr != sliced) ? sliced->GetSlices() : 1u;
            auto& sliceMap = info.decayTimes[t];
            for (unsigned int s = 0; s < slices; ++s)
            {
              const std::string acqDate = acqDateProp->GetValue(t, s, true, true);
              const std::string acqTime = acqTimeProp->GetValue(t, s, true, true);
              OFDateTime ofAcq;
              if (!ConvertDICOMDateTimeString(acqDate, acqTime, ofAcq))
              {
                mitkThrowException(InvalidDICOMPropertyValueException)
                  << "Cannot parse acquisition Date+Time '" << acqDate << acqTime
                  << "' at timestep " << t << " slice " << s << ".";
              }
              sliceMap[static_cast<SlicedData::IndexValueType>(s)] =
                ResolveDecayDurationSeconds(admin, ofAcq, 0.0, halfLife,
                                            policy, info.adaptations);
            }
          }
          return info;
        }
      }

      // ---- Steps 3/4: general T_ave formula, or the GE offset, per slice ----
      // Both require per-slice (0008,0032) AcqTime and (0054,0x1300)
      // FrameReferenceTime. Step 3 additionally needs (0018,0x1242)
      // ActualFrameDuration and a half-life, because its T_ave term is
      // computed from them; Step 4 is a pure shift and needs neither.
      // We require the preconditions across all slices; if any slice is
      // incomplete we fall through rather than mix per-slice formulas with
      // a partial result.
      const auto* frameRefProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0054, 0x1300));
      const auto* frameDurProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0018, 0x1242));

      // Only the GE rule is manufacturer-specific. Siemens, Philips and any
      // manufacturer we cannot classify share the general T_ave form, so the
      // split is "GE versus everything else" rather than an allow-list.
      const bool isStep4 = (ManufacturerFamily::GE == manuf);
      const bool isStep3 = !isStep4;
      const bool halfLifeOK = std::isfinite(halfLife) && halfLife > 0.0;
      const bool step3Possible = isStep3 && halfLifeOK && nullptr != frameDurProp;
      const bool step4Possible = isStep4;

      // Steps 3 and 4 are vendor-specific empirical formulas (not derivable
      // from the DICOM spec alone). Under Strict policy we refuse them and
      // surface the input ambiguity so the caller can supply timing
      // out-of-band; under Lenient policy we apply them with the
      // benchmark-recommended formula.
      const bool stepPossible = haveAcqTags && nullptr != frameRefProp
                             && (step3Possible || step4Possible);

      if (stepPossible && DICOMReadPolicy::Strict == policy)
      {
        mitkThrowException(VendorEmpiricalDecayFallbackRefusedException)
          << "DC=START Steps 1 and 2 do not apply to this input "
             "(no vendor private decay datetime, AcquisitionTime != "
             "SeriesTime). Steps 3 / 4 would resolve the reference time "
             "via a vendor-specific empirical formula, but "
             "DICOMReadPolicy::Strict is active. Re-export the data with "
             "an unambiguous reference (vendor private datetime, or "
             "AcquisitionTime aligned with SeriesTime), or supply timing "
             "via --decay-time / a manual decay-time override.";
      }

      if (stepPossible)
      {
        const auto timeSteps = data->GetTimeSteps();
        DecayTimeMapType candidateMap;
        bool allSlicesValid = true;

        for (TimeStepType t = 0; t < timeSteps && allSlicesValid; ++t)
        {
          const auto* sliced = data->GetSlicedGeometry(t);
          const unsigned int slices = (nullptr != sliced) ? sliced->GetSlices() : 1u;
          auto& sliceMap = candidateMap[t];

          for (unsigned int s = 0; s < slices && allSlicesValid; ++s)
          {
            const std::string acqDate = acqDateProp->GetValue(t, s, true, true);
            const std::string acqTime = acqTimeProp->GetValue(t, s, true, true);
            OFDateTime ofAcq;
            if (!ConvertDICOMDateTimeString(acqDate, acqTime, ofAcq))
            {
              allSlicesValid = false;
              break;
            }

            // (0054,0x1300) FrameReferenceTime and (0018,0x1242)
            // ActualFrameDuration are stored in milliseconds per DICOM.
            // Spec precondition: FrameReferenceTime non-negative.
            const double frameRefMs = ReadNumericTagAt(frameRefProp, t, s);
            if (!std::isfinite(frameRefMs) || frameRefMs < 0.0)
            {
              allSlicesValid = false;
              break;
            }
            const double frameRefSec = frameRefMs / 1000.0;

            // Step 3 (general): t_ref = AcqTime + T_ave - FrameReferenceTime
            // Step 4 (GE):      t_ref = AcqTime - FrameReferenceTime
            // The -FrameReferenceTime term undoes the scanner-applied offset
            // from the per-frame midpoint back to the start of acquisition;
            // T_ave additionally compensates for the average count-rate time
            // inside the frame. Without the FrameReferenceTime term, Step 3
            // diverges from IBSI-SUV expectations by exactly that offset
            // (verified against DRO_3_2_0 / DRO_3_2_2).
            //
            // ActualFrameDuration is read only on the Step 3 branch: it
            // feeds T_ave and nothing else, so requiring it on the GE path
            // would refuse inputs that the GE rule resolves perfectly well.
            double offset = -frameRefSec;
            if (step3Possible)
            {
              const double frameDurMs = ReadNumericTagAt(frameDurProp, t, s);
              // Spec precondition: ActualFrameDuration strictly positive.
              if (!std::isfinite(frameDurMs) || frameDurMs <= 0.0)
              {
                allSlicesValid = false;
                break;
              }
              offset = ComputeTAveSeconds(frameDurMs / 1000.0, halfLife) - frameRefSec;
            }

            sliceMap[static_cast<SlicedData::IndexValueType>(s)] =
              ResolveDecayDurationSeconds(admin, ofAcq, offset, halfLife,
                                          policy, info.adaptations);
          }
        }

        if (allSlicesValid)
        {
          // Announce the adaptation only once it has actually been applied.
          // Warning earlier would cry wolf on inputs that fall through to
          // the exhaustion refusal below.
          const char* const stepName = step3Possible
            ? "Step 3 (AcquisitionTime + T_ave - FrameReferenceTime)"
            : "Step 4 (AcquisitionTime - FrameReferenceTime)";
          MITK_WARN << "DC=START reference time resolved via " << stepName
                    << ". This formula is derived from observed scanner "
                       "behaviour, not from the DICOM specification; the "
                       "strict DICOM read policy refuses it.";
          info.adaptations.push_back(
            {SUVAdaptationRule::VendorEmpiricalDecayFallback, "", "", stepName});

          if (ManufacturerFamily::Other == manuf)
          {
            const std::string rawManufacturer =
              mitk::GetFirstDICOMValueAsString(data, DICOMTagPath(0x0008, 0x0070));
            MITK_WARN << "(0008,0070) Manufacturer '" << rawManufacturer
                      << "' is not one of the manufacturers this formula was "
                         "validated against. The general rule was applied; "
                         "treat the resulting decay timing with caution.";
            info.adaptations.push_back({SUVAdaptationRule::UnrecognizedManufacturer,
                                        "(0008,0070)", rawManufacturer, stepName});
          }

          info.decayTimes = std::move(candidateMap);
          return info;
        }
      }

      // ---- Step 5: spec is silent. Refuse to extend a vendor formula to
      //              an unclassifiable input or to silently fall back to
      //              SeriesTime.
      mitkThrowException(AmbiguousDecayTimingException)
        << "DC=START fallback chain exhausted: manufacturer '"
        << mitk::GetFirstDICOMValueAsString(data, DICOMTagPath(0x0008, 0x0070))
        << "' / available DICOM input does not match any of the "
           "IBSI-SUV-recommended reference-time paths. Required tags for "
           "the vendor-aware paths: Siemens (0071,0x22) / GE (0009,0x0D) "
           "private datetime, or per-slice (0008,0032) AcquisitionTime "
           "equal to (0008,0031) SeriesTime, or per-slice (0008,0032) + "
           "(0054,0x1300) + (0018,0x1242). Supply --decay-time / a "
           "manual decay-time override at the consuming layer to bypass "
           "DICOM-derived computation.";
    }

    case DecayCorrectionStrategy::None:
    {
      // Spec: voxel values of non-decay-corrected images correspond to
      // (AcquisitionTime + T_ave); decay-correct the dose to that time
      // point. T_ave depends on per-slice (0018,0x1242) ActualFrameDuration
      // and the half-life. Inputs that lack these tags are rejected — the
      // approximation (t_acq - t_inj) silently introduces a ~3 % bias for
      // typical 5-minute frames and is precisely the bug we are fixing.
      // Use the consuming layer's decay-time override (e.g. CLI
      // --decay-time) for inputs whose timing must be supplied externally.
      const auto* acqDateProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0008, 0x0022));
      const auto* acqTimeProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0008, 0x0032));
      const auto* frameDurProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0018, 0x1242));
      if (nullptr == acqDateProp)
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Strategy NONE requires (0008,0022) Acquisition Date per slice. "
             "Tag is missing.";
      }
      if (nullptr == acqTimeProp)
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Strategy NONE requires (0008,0032) Acquisition Time per slice. "
             "Tag is missing.";
      }
      if (nullptr == frameDurProp)
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Strategy NONE requires (0018,0x1242) Actual Frame Duration per "
             "slice for the IBSI-SUV-recommended T_ave correction "
             "((t_acq + T_ave) - t_inj). Tag is missing; supply a manual "
             "decay-time override to bypass DICOM-derived computation.";
      }

      const double halfLife = ResolveHalfLifeSeconds(data, halfLifeSeconds);
      if (!std::isfinite(halfLife) || halfLife <= 0.0)
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Strategy NONE requires a positive radionuclide half-life for "
             "the T_ave correction. Provide it via --half-life / --nuclide "
             "or supply DICOM (0018,1075).";
      }

      // Administration tags are image-level; resolve once and reuse across
      // all slices / timesteps.
      const auto admin = ResolveAdministrationTimeTags(data);

      const auto timeSteps = data->GetTimeSteps();
      for (TimeStepType t = 0; t < timeSteps; ++t)
      {
        const auto* sliced = data->GetSlicedGeometry(t);
        const unsigned int slices = (nullptr != sliced) ? sliced->GetSlices() : 1u;
        auto& sliceMap = info.decayTimes[t];
        for (unsigned int s = 0; s < slices; ++s)
        {
          const std::string acqDate = acqDateProp->GetValue(t, s, true, true);
          const std::string acqTime = acqTimeProp->GetValue(t, s, true, true);

          OFDateTime ofAcq;
          if (!ConvertDICOMDateTimeString(acqDate, acqTime, ofAcq))
          {
            mitkThrowException(InvalidDICOMPropertyValueException)
              << "Cannot parse acquisition Date+Time '" << acqDate << acqTime
              << "' at timestep " << t << " slice " << s << ".";
          }

          // (0018,0x1242) ActualFrameDuration is stored in milliseconds.
          const double frameDurMs = ReadNumericTagAt(frameDurProp, t, s);
          if (!std::isfinite(frameDurMs) || frameDurMs <= 0.0)
          {
            mitkThrowException(InvalidDICOMPropertyValueException)
              << "Strategy NONE: (0018,0x1242) Actual Frame Duration at "
                 "timestep " << t << " slice " << s
              << " is missing or non-positive (got " << frameDurMs << " ms).";
          }
          const double tAveSec = ComputeTAveSeconds(frameDurMs / 1000.0, halfLife);

          sliceMap[static_cast<SlicedData::IndexValueType>(s)] =
            ResolveDecayDurationSeconds(admin, ofAcq, tAveSec, halfLife,
                                        policy, info.adaptations);
        }
      }
      return info;
    }

    case DecayCorrectionStrategy::Manual:
    {
      mitkThrow() << "DeduceDecayCorrection cannot return Manual strategy; "
                     "Manual is set by callers that provide a decay time directly.";
    }
  }

  // Unreachable: GetDecayCorrectionStrategy throws on unknown values.
  mitkThrow() << "DeduceDecayCorrection: unhandled DecayCorrectionStrategy value.";
}
