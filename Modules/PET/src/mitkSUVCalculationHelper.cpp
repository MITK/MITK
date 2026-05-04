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

namespace
{
  // Iterate the property keys of an IPropertyProvider and return those whose
  // names parse back into a DICOMTagPath equal to the given path. This keeps
  // MitkDICOM untouched: only PropertyList* and BaseData* overloads of
  // mitk::GetPropertyByDICOMTagPath ship there, and we want to take
  // IPropertyProvider* without forcing a dependency change in MitkDICOM.
  std::map<std::string, mitk::BaseProperty::ConstPointer>
  CollectPropertiesByDICOMTagPath(const mitk::IPropertyProvider* provider,
                                  const mitk::DICOMTagPath& path)
  {
    std::map<std::string, mitk::BaseProperty::ConstPointer> result;

    if (nullptr == provider)
    {
      return result;
    }

    for (const auto& key : provider->GetPropertyKeys())
    {
      const mitk::DICOMTagPath propPath = mitk::PropertyNameToDICOMTagPath(key);
      if (!propPath.IsEmpty() && path.Equals(propPath))
      {
        auto prop = provider->GetConstProperty(key);
        if (prop.IsNotNull())
        {
          result[key] = prop;
        }
      }
    }
    return result;
  }

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
  // index by (timestep, slice). Returns nullptr if no match.
  const mitk::DICOMProperty* FindFirstDICOMProperty(
    const mitk::IPropertyProvider* provider,
    const mitk::DICOMTagPath& path,
    std::string* outKey = nullptr)
  {
    auto matches = CollectPropertiesByDICOMTagPath(provider, path);
    if (matches.empty())
    {
      return nullptr;
    }
    if (matches.size() > 1)
    {
      MITK_WARN << "Multiple properties match DICOM tag path " << path.ToStr()
                << "; using the first match: " << matches.begin()->first;
    }
    if (outKey)
    {
      *outKey = matches.begin()->first;
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

  boost::posix_time::ptime ConvertOFDateTimeToPTime(const OFDateTime& time)
  {
    const boost::gregorian::date boostDate(
      time.getDate().getYear(), time.getDate().getMonth(), time.getDate().getDay());

    const boost::posix_time::time_duration boostTime =
      boost::posix_time::hours(time.getTime().getHour())
      + boost::posix_time::minutes(time.getTime().getMinute())
      + boost::posix_time::seconds(time.getTime().getIntSecond())
      + boost::posix_time::milliseconds(time.getTime().getMilliSecond());

    return boost::posix_time::ptime(boostDate, boostTime);
  }

  // Returns (reference - injection) in seconds.
  double DurationInSeconds(const OFDateTime& injection, const OFDateTime& reference)
  {
    const auto pInjection = ConvertOFDateTimeToPTime(injection);
    const auto pReference = ConvertOFDateTimeToPTime(reference);
    const boost::posix_time::time_duration delta = pReference - pInjection;
    return static_cast<double>(delta.total_milliseconds()) / 1000.0;
  }

  // Resolve the radiopharmaceutical injection time into an absolute OFDateTime.
  // Returns (parsed time, derivedFromTimeOnlyTag). The bool tells the caller
  // whether the rollover guard may safely subtract 24 h to recover an
  // ambiguous timing situation.
  std::pair<OFDateTime, bool> ResolveInjectionDateTime(
    const mitk::IPropertyProvider* provider,
    const std::string& fallbackAcquisitionDate)
  {
    // Prefer the unambiguous DateTime tag.
    mitk::DICOMTagPath startDateTimePath;
    startDateTimePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1078);
    const auto* startDateTimeProp = FindFirstDICOMProperty(provider, startDateTimePath);

    if (startDateTimeProp != nullptr)
    {
      OFDateTime parsed;
      // (0018,1078) is a DT VR — date and time are already encoded together.
      if (!ConvertDICOMDateTimeString("", startDateTimeProp->GetValue(0, 0, true, true), parsed))
      {
        mitkThrowException(mitk::InvalidDICOMPropertyValueException)
          << "Cannot parse Radiopharmaceutical Start DateTime (0018,1078) value '"
          << startDateTimeProp->GetValue(0, 0, true, true) << "'.";
      }
      return { parsed, false };
    }

    mitk::DICOMTagPath startTimePath;
    startTimePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1072);
    const auto* startTimeProp = FindFirstDICOMProperty(provider, startTimePath);

    if (startTimeProp != nullptr)
    {
      if (fallbackAcquisitionDate.empty())
      {
        mitkThrowException(mitk::MissingDICOMPropertyException)
          << "Radiopharmaceutical Start Time (0018,1072) is present but no "
             "fallback acquisition date is available to construct an absolute "
             "injection timestamp.";
      }
      OFDateTime parsed;
      if (!ConvertDICOMDateTimeString(fallbackAcquisitionDate,
                                      startTimeProp->GetValue(0, 0, true, true),
                                      parsed))
      {
        mitkThrowException(mitk::InvalidDICOMPropertyValueException)
          << "Cannot parse Radiopharmaceutical Start Time (0018,1072) value '"
          << startTimeProp->GetValue(0, 0, true, true)
          << "' with fallback date '" << fallbackAcquisitionDate << "'.";
      }
      return { parsed, true };
    }

    mitkThrowException(mitk::MissingDICOMPropertyException)
      << "No radiopharmaceutical injection time available: neither "
         "(0018,1078) Radiopharmaceutical Start DateTime nor (0018,1072) "
         "Radiopharmaceutical Start Time was found.";
  }

  // Compute (reference - injection) in seconds with rollover handling.
  // - If the result is negative AND the injection time was derived from
  //   the (0018,1072) TM-only tag, subtract 24 h from the injection and try
  //   once more. This recovers the typical "injected last night, scanned
  //   this morning" ambiguity.
  // - If the result is still outside [0, 24 h], throw
  //   AmbiguousDecayTimingException.
  // - For (0018,1078)-derived data, no rollover correction is applied; any
  //   negative duration throws.
  double ComputeDecayTimeWithRolloverGuard(const OFDateTime& injection,
                                           const OFDateTime& reference,
                                           bool injectionFromTimeOnlyTag)
  {
    constexpr double kSecondsIn24h = 24.0 * 60.0 * 60.0;

    double seconds = DurationInSeconds(injection, reference);

    if (seconds < 0.0 && injectionFromTimeOnlyTag)
    {
      const auto pAdjustedInjection = ConvertOFDateTimeToPTime(injection)
        - boost::posix_time::hours(24);
      const auto pReference = ConvertOFDateTimeToPTime(reference);
      const boost::posix_time::time_duration delta = pReference - pAdjustedInjection;
      seconds = static_cast<double>(delta.total_milliseconds()) / 1000.0;
    }

    if (seconds < 0.0 || seconds > kSecondsIn24h)
    {
      mitkThrowException(mitk::AmbiguousDecayTimingException)
        << "Cannot reconcile radiopharmaceutical injection time and "
           "acquisition / series reference time. Computed decay duration: "
        << seconds << " s. Please re-export the data with "
           "(0018,1078) Radiopharmaceutical Start DateTime to remove the "
           "ambiguity.";
    }

    return seconds;
  }

  std::string ReadStringTag(const mitk::IPropertyProvider* provider,
                            const mitk::DICOMTagPath& path)
  {
    const auto* prop = FindFirstDICOMProperty(provider, path);
    if (prop == nullptr)
    {
      return {};
    }
    return prop->GetValue(0, 0, true, true);
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
mitk::GetRadiopharmaceuticalInfos(const mitk::IPropertyProvider* provider)
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
    const auto matches = CollectPropertiesByDICOMTagPath(provider, queryPath);
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

  // Total dose (0018,1074).
  DICOMTagPath dosePath;
  dosePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1074);
  enumerate(dosePath, [](RadiopharmaceuticalInfo& info, const std::string& v)
  {
    info.totalDoseBq = ConvertDICOMStrToValue<double>(v);
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
  const auto props = CollectPropertiesByDICOMTagPath(provider, weightPath);

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
  const auto props = CollectPropertiesByDICOMTagPath(provider, heightPath);

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
  const std::string raw = ReadStringTag(provider, sexPath);
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

mitk::DecayCorrectionStrategy mitk::GetDecayCorrectionStrategy(const mitk::IPropertyProvider* provider)
{
  const DICOMTagPath decayCorrPath(0x0054, 0x1102);

  const std::string raw = ReadStringTag(provider, decayCorrPath);
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
      ReadStringTag(provider, mitk::DICOMTagPath(0x0008, 0x0021));
    if (!seriesDate.empty())
    {
      return seriesDate;
    }
    return ReadStringTag(provider, mitk::DICOMTagPath(0x0008, 0x0022));
  }
}

mitk::DecayCorrectionInfo mitk::DeduceDecayCorrection(const mitk::SlicedData* data)
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
      const std::string referenceDate = ResolveSeriesReferenceDate(data);
      if (referenceDate.empty())
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Strategy START requires a series / acquisition date "
             "(0008,0021 or 0008,0022) to assemble an absolute Series Time. "
             "Neither tag was found.";
      }

      const std::string seriesTime =
        ReadStringTag(data, DICOMTagPath(0x0008, 0x0031));
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

      const auto injection = ResolveInjectionDateTime(data, referenceDate);
      const double decayTime =
        ComputeDecayTimeWithRolloverGuard(injection.first, ofSeriesTime, injection.second);

      FillUniformDecayMap(data, decayTime, info.decayTimes);
      return info;
    }

    case DecayCorrectionStrategy::None:
    {
      const auto* acqDateProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0008, 0x0022));
      const auto* acqTimeProp = FindFirstDICOMProperty(data, DICOMTagPath(0x0008, 0x0032));
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

      // Injection time is image-level; resolve once and reuse across all slices/timesteps.
      const std::string firstSliceAcqDate = acqDateProp->GetValue(0, 0, true, true);
      const auto injection = ResolveInjectionDateTime(data, firstSliceAcqDate);

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
            ComputeDecayTimeWithRolloverGuard(injection.first, ofAcq, injection.second);
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
