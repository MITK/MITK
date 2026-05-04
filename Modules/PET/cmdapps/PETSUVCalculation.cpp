/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// std includes
#include <algorithm>
#include <cctype>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

// itk includes
#include <itkImage.h>
#include <itkIndex.h>

// CTK includes
#include <mitkCommandLineParser.h>

// MITK Core / DICOM
#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkITKImageImport.h>
#include <mitkLog.h>
#include <mitkPixelType.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkProperties.h>

// MITK PET
#include <itkIndexedUnaryFunctorImageFilter.h>
#include <mitkHalfLifeConstants.h>
#include <mitkSUVCalculation.h>
#include <mitkSUVCalculationHelper.h>
#include <mitkSUVFunctorPolicy.h>
#include <mitkSUVNormalizationStrategy.h>


namespace
{
  // Exit codes used by the CLI so calling scripts can branch on the
  // reason for failure rather than parse log messages.
  enum class ExitCode : int
  {
    Success                  = 0,
    Generic                  = 1,
    MissingDICOMProperty     = 2,
    AmbiguousDecayTiming     = 3,
    InvalidArguments         = 4,
    MultiTracerWithoutIndex  = 5,
    InvalidDICOMPropertyValue= 6,
    MissingSUVInput          = 7,
  };

  int AsInt(ExitCode c) { return static_cast<int>(c); }

  // Settings populated from the CLI. Optionals carry "user-supplied
  // override or use DICOM" semantics.
  struct Settings
  {
    std::string inFileName;
    std::string outFileName;

    mitk::SUVVariant variant = mitk::SUVVariant::BW;

    std::optional<double>     injectedActivityBq;
    std::optional<double>     bodyWeightKg;
    std::optional<double>     heightM;
    std::optional<mitk::Sex>  sex;
    std::optional<double>     halfLifeS;
    std::optional<double>     decayTimeS;     // uniform decay-time override
    std::optional<int>        tracerIndex;    // explicit RPI item selection

    bool ignoreModalityCheck = false;
    bool ignoreUnitsCheck    = false;
    bool verbose             = false;
  };

  std::string TrimAsciiWhitespace(const std::string& s)
  {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
  }

  std::string ToUpperAscii(std::string s)
  {
    std::transform(s.begin(), s.end(), s.begin(),
      [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
  }

  std::optional<mitk::SUVVariant> ParseVariant(const std::string& raw)
  {
    const std::string v = ToUpperAscii(TrimAsciiWhitespace(raw));
    if ("BW"  == v) return mitk::SUVVariant::BW;
    if ("LBM" == v) return mitk::SUVVariant::LBM;
    if ("BSA" == v) return mitk::SUVVariant::BSA;
    return std::nullopt;
  }

  std::optional<mitk::Sex> ParseSex(const std::string& raw)
  {
    const std::string v = ToUpperAscii(TrimAsciiWhitespace(raw));
    if ("M" == v) return mitk::Sex::Male;
    if ("F" == v) return mitk::Sex::Female;
    return std::nullopt;
  }

  std::optional<double> ResolveNuclideHalfLife(const std::string& raw)
  {
    const std::string n = TrimAsciiWhitespace(raw);
    if (n == mitk::HALFLIFECONSTANTS::NAME_18F)  return mitk::HALFLIFECONSTANTS::VALUE_18F;
    if (n == mitk::HALFLIFECONSTANTS::NAME_68Ga) return mitk::HALFLIFECONSTANTS::VALUE_68Ga;
    if (n == mitk::HALFLIFECONSTANTS::NAME_11C)  return mitk::HALFLIFECONSTANTS::VALUE_11C;
    if (n == mitk::HALFLIFECONSTANTS::NAME_15O)  return mitk::HALFLIFECONSTANTS::VALUE_15O;
    return std::nullopt;
  }

  std::string ReadFirstStringTag(const mitk::BaseData* data, const mitk::DICOMTagPath& path)
  {
    const auto props = mitk::GetPropertyByDICOMTagPath(data, path);
    if (props.empty()) return {};
    auto* dicomProp = dynamic_cast<const mitk::DICOMProperty*>(props.begin()->second.GetPointer());
    if (nullptr == dicomProp) return {};
    return dicomProp->GetValue(0, 0, true, true);
  }

  void setupParser(mitkCommandLineParser& parser)
  {
    parser.setCategory("PET Tools");
    parser.setTitle("PET SUV Calculation");
    parser.setDescription(
      "CLI app that converts a PET image (in [Bq/mL]) into a SUV image. "
      "Supports body-weight (SUVbw), lean-body-mass (SUVlbm, Janmahasatian), "
      "and body-surface-area (SUVbsa, DuBois) normalization. Acquisition "
      "parameters (injected dose, half-life, decay timing, patient body "
      "weight / height / sex) are taken from the input's DICOM properties "
      "by default and can be overridden via dedicated flags.");
    parser.setContributor("DKFZ MIC");

    parser.setArgumentPrefix("--", "-");

    parser.beginGroup("Required I/O parameters");
    parser.addArgument("input", "i", mitkCommandLineParser::File,
      "Input image", "Path to the input PET image (DICOM dir or supported MITK format).",
      us::Any(), false, false, false, mitkCommandLineParser::Input);
    parser.addArgument("output", "o", mitkCommandLineParser::File,
      "Output file", "Where to save the resulting SUV image (e.g. .nrrd).",
      us::Any(), false, false, false, mitkCommandLineParser::Output);
    parser.endGroup();

    parser.beginGroup("Variant selection");
    parser.addArgument("variant", "", mitkCommandLineParser::String,
      "SUV variant",
      "One of: bw (body weight, default), lbm (lean body mass, Janmahasatian), "
      "bsa (body surface area, DuBois).",
      us::Any(std::string("bw")));
    parser.endGroup();

    parser.beginGroup("Acquisition parameter overrides");
    parser.addArgument("injected-activity", "", mitkCommandLineParser::Float,
      "Injected activity [Bq]",
      "Override DICOM (0018,1074) Radionuclide Total Dose.");
    parser.addArgument("body-weight", "", mitkCommandLineParser::Float,
      "Body weight [kg]",
      "Override DICOM (0010,1030) Patient Weight.");
    parser.addArgument("patient-height", "", mitkCommandLineParser::Float,
      "Patient height [m]",
      "Override DICOM (0010,1020) Patient Size. Required for variant lbm/bsa.");
    parser.addArgument("patient-sex", "", mitkCommandLineParser::String,
      "Patient sex (M|F)",
      "Override DICOM (0010,0040) Patient Sex. Required for variant lbm.");
    parser.addArgument("half-life", "", mitkCommandLineParser::Float,
      "Half-life [s]",
      "Override DICOM (0018,1075) Radionuclide Half-Life.");
    parser.addArgument("nuclide", "", mitkCommandLineParser::String,
      "Radionuclide name",
      "Alternative to --half-life: one of 18F, 68Ga, 11C, 15O.");
    parser.addArgument("decay-time", "", mitkCommandLineParser::Float,
      "Uniform decay time [s]",
      "Override the per-voxel decay times derived from DICOM. Applied "
      "uniformly to all voxels. Intended for inputs without DICOM timing "
      "tags, or to reproduce ADMIN-style behaviour by passing 0; not a "
      "substitute for the per-slice handling DICOM strategy NONE applies.");
    parser.endGroup();

    parser.beginGroup("Validation switches");
    parser.addArgument("ignore-modality-check", "", mitkCommandLineParser::Bool,
      "Ignore modality check",
      "Bypass the (0008,0060) Modality == 'PT' check.");
    parser.addArgument("ignore-units-check", "", mitkCommandLineParser::Bool,
      "Ignore units check",
      "Bypass the (0054,1001) Units == 'BQML' check.");
    parser.addArgument("tracer-index", "", mitkCommandLineParser::Int,
      "Radiopharmaceutical sequence item index",
      "Explicit selection for multi-item Radiopharmaceutical Information "
      "Sequence (0054,0016). Without this flag, multi-item sequences are "
      "rejected.");
    parser.endGroup();

    parser.beginGroup("Optional");
    parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Verbose", "Verbose output.");
    parser.addArgument("help",    "h", mitkCommandLineParser::Bool, "Help",    "Show this help text.");
    parser.endGroup();
  }

  bool configureSettings(const std::map<std::string, us::Any>& parsed, Settings& s)
  {
    if (parsed.empty()) return false;

    try
    {
      s.inFileName  = us::any_cast<std::string>(parsed.at("input"));
      s.outFileName = us::any_cast<std::string>(parsed.at("output"));
    }
    catch (...)
    {
      return false;
    }

    if (parsed.count("variant"))
    {
      const auto raw = us::any_cast<std::string>(parsed.at("variant"));
      const auto v   = ParseVariant(raw);
      if (!v.has_value())
      {
        MITK_ERROR << "Invalid --variant value '" << raw << "'. Expected one of bw, lbm, bsa.";
        return false;
      }
      s.variant = v.value();
    }

    if (parsed.count("injected-activity")) s.injectedActivityBq = us::any_cast<float>(parsed.at("injected-activity"));
    if (parsed.count("body-weight"))       s.bodyWeightKg       = us::any_cast<float>(parsed.at("body-weight"));
    if (parsed.count("patient-height"))    s.heightM            = us::any_cast<float>(parsed.at("patient-height"));
    if (parsed.count("half-life"))         s.halfLifeS          = us::any_cast<float>(parsed.at("half-life"));
    if (parsed.count("decay-time"))        s.decayTimeS         = us::any_cast<float>(parsed.at("decay-time"));
    if (parsed.count("tracer-index"))      s.tracerIndex        = us::any_cast<int>(parsed.at("tracer-index"));

    if (parsed.count("patient-sex"))
    {
      const auto raw = us::any_cast<std::string>(parsed.at("patient-sex"));
      const auto sx  = ParseSex(raw);
      if (!sx.has_value())
      {
        MITK_ERROR << "Invalid --patient-sex value '" << raw << "'. Expected M or F.";
        return false;
      }
      s.sex = sx.value();
    }

    if (parsed.count("nuclide"))
    {
      const auto raw = us::any_cast<std::string>(parsed.at("nuclide"));
      const auto hl  = ResolveNuclideHalfLife(raw);
      if (!hl.has_value())
      {
        MITK_ERROR << "Unknown --nuclide '" << raw << "'. Expected one of 18F, 68Ga, 11C, 15O.";
        return false;
      }
      // CLI flag --half-life takes precedence over --nuclide.
      if (!s.halfLifeS.has_value())
      {
        s.halfLifeS = hl.value();
      }
    }

    if (parsed.count("ignore-modality-check")) s.ignoreModalityCheck = us::any_cast<bool>(parsed.at("ignore-modality-check"));
    if (parsed.count("ignore-units-check"))    s.ignoreUnitsCheck    = us::any_cast<bool>(parsed.at("ignore-units-check"));
    if (parsed.count("verbose"))               s.verbose             = us::any_cast<bool>(parsed.at("verbose"));

    return true;
  }

  // Validate (0008,0060) Modality == "PT" (case-insensitive, trimmed).
  // Returns true if check passes (or is bypassed); false if the CLI
  // should abort.
  bool ValidateModality(const mitk::Image* image, bool ignore)
  {
    const std::string raw = ReadFirstStringTag(image, mitk::DICOMTagPath(0x0008, 0x0060));
    const std::string normalized = ToUpperAscii(TrimAsciiWhitespace(raw));
    if ("PT" == normalized) return true;

    if (ignore)
    {
      MITK_WARN << "Modality (0008,0060) = '" << raw
                << "' (expected PT). Continuing because --ignore-modality-check is set.";
      return true;
    }
    MITK_ERROR << "Modality (0008,0060) = '" << raw
               << "' (expected PT). Use --ignore-modality-check to bypass.";
    return false;
  }

  // Validate (0054,1001) Units == "BQML" (trimmed, uppercased; exact match).
  bool ValidateUnits(const mitk::Image* image, bool ignore)
  {
    const std::string raw = ReadFirstStringTag(image, mitk::DICOMTagPath(0x0054, 0x1001));
    const std::string normalized = ToUpperAscii(TrimAsciiWhitespace(raw));
    if ("BQML" == normalized) return true;

    if (ignore)
    {
      MITK_WARN << "Units (0054,1001) = '" << raw
                << "' (expected BQML). Continuing because --ignore-units-check is set; "
                << "the resulting SUV will not be physically meaningful.";
      return true;
    }
    MITK_ERROR << "Units (0054,1001) = '" << raw
               << "' (expected BQML). Use --ignore-units-check to bypass.";
    return false;
  }

  // Pick a single Radiopharmaceutical Information Sequence item, honouring
  // CLI overrides for activity/half-life. Returns the resolved index or
  // an exit code via the throw-int convention used only inside main.
  struct TracerSelection
  {
    int                                       index = -1;            // -1 = no item available, all from overrides
    mitk::RadiopharmaceuticalInfo             info;                  // empty if index == -1
  };

  TracerSelection SelectTracer(const std::vector<mitk::RadiopharmaceuticalInfo>& infos,
                               const Settings& s,
                               ExitCode& outExitCode)
  {
    TracerSelection sel;

    if (infos.empty())
    {
      // No DICOM RPI sequence available. Caller must have provided overrides
      // for both injected activity and half-life — the resolution step
      // checks for that and reports a clear message.
      outExitCode = ExitCode::Success;
      sel.index = -1;
      return sel;
    }

    if (infos.size() == 1 && !s.tracerIndex.has_value())
    {
      outExitCode = ExitCode::Success;
      sel.index = 0;
      sel.info  = infos[0];
      return sel;
    }

    if (s.tracerIndex.has_value())
    {
      const int idx = s.tracerIndex.value();
      if (idx < 0 || static_cast<std::size_t>(idx) >= infos.size())
      {
        MITK_ERROR << "--tracer-index " << idx << " is out of range. Sequence has "
                   << infos.size() << " item(s).";
        outExitCode = ExitCode::InvalidArguments;
        sel.index = -2;
        return sel;
      }
      outExitCode = ExitCode::Success;
      sel.index = idx;
      sel.info  = infos[idx];
      return sel;
    }

    // Multi-item sequence and no explicit selection: refuse.
    std::ostringstream names;
    for (std::size_t i = 0; i < infos.size(); ++i)
    {
      if (i > 0) names << ", ";
      names << "[" << i << "] " << (infos[i].name.empty() ? "<unnamed>" : infos[i].name);
    }
    MITK_ERROR << "Input contains a multi-item Radiopharmaceutical Information "
                  "Sequence (0054,0016). Pass --tracer-index N to select one. "
                  "Items: " << names.str();
    outExitCode = ExitCode::MultiTracerWithoutIndex;
    sel.index = -2;
    return sel;
  }

  // Densify the per-(timestep, slice) decay-time map for one timestep
  // into a vector indexed by slice index (idx[2]). The lambda inside the
  // pipeline then becomes O(1) per voxel rather than O(log slices).
  std::vector<double> DensifyDecayTimes(const mitk::DecayTimeSliceMapType& sliceMap,
                                        std::size_t expectedSlices)
  {
    std::vector<double> result(expectedSlices,
                               std::numeric_limits<double>::quiet_NaN());
    for (const auto& kv : sliceMap)
    {
      const auto idx = static_cast<std::size_t>(kv.first);
      if (idx < expectedSlices)
      {
        result[idx] = kv.second;
      }
    }
    return result;
  }

  // Apply the SUV functor to a single 3D timestep input image and write
  // the result into the destination 4D output at \p dstStep.
  void ProcessTimeStep(const mitk::Image* stepIn,
                       mitk::Image* dst,
                       mitk::TimeStepType dstStep,
                       double injectedActivity,
                       double scaleNumerator,
                       double halfLife,
                       const std::vector<double>& sliceDecay)
  {
    using ImageT = itk::Image<double, 3>;
    ImageT::Pointer itkIn;
    mitk::CastToItkImage(stepIn, itkIn);

    mitk::SUVFunctorPolicy functor(injectedActivity, scaleNumerator, halfLife);
    functor.SetDecayTimeFunctor(
      [&sliceDecay](const itk::Index<3>& idx) {
        const auto z = static_cast<std::size_t>(idx[2]);
        return (z < sliceDecay.size())
          ? sliceDecay[z]
          : std::numeric_limits<double>::quiet_NaN();
      });

    if (!functor.IsConfigured())
    {
      mitkThrow() << "SUV functor is not fully configured (NaN scalar parameter).";
    }

    using FilterT = itk::IndexedUnaryFunctorImageFilter<ImageT, ImageT, mitk::SUVFunctorPolicy>;
    auto filter = FilterT::New();
    filter->SetFunctor(functor);
    filter->SetInput(itkIn);
    filter->Update();

    auto suvSlab = mitk::ImportItkImage(filter->GetOutput());
    mitk::ImageReadAccessor acc(suvSlab);
    dst->SetVolume(acc.GetData(), dstStep);
  }
}


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  setupParser(parser);

  const auto parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return AsInt(ExitCode::Success);
  }

  Settings s;
  if (!configureSettings(parsedArgs, s))
  {
    MITK_ERROR << "Invalid command-line arguments. Use --help for usage.";
    return AsInt(ExitCode::InvalidArguments);
  }

  try
  {
    // ---- Load -----------------------------------------------------------

    if (s.verbose) MITK_INFO << "Loading input: " << s.inFileName;
    mitk::PreferenceListReaderOptionsFunctor readerFunctor(
      { "MITK DICOM Reader v2 (autoselect)" }, { "" });
    auto image = mitk::IOUtil::Load<mitk::Image>(s.inFileName, &readerFunctor);
    if (image.IsNull())
    {
      MITK_ERROR << "Cannot load input image: " << s.inFileName;
      return AsInt(ExitCode::Generic);
    }

    // ---- Validate modality / units -------------------------------------

    if (!ValidateModality(image, s.ignoreModalityCheck))
    {
      return AsInt(ExitCode::InvalidArguments);
    }
    if (!ValidateUnits(image, s.ignoreUnitsCheck))
    {
      return AsInt(ExitCode::InvalidArguments);
    }

    // ---- Resolve scalar parameters --------------------------------------

    auto rpiInfos = mitk::GetRadiopharmaceuticalInfos(image.GetPointer());

    ExitCode tracerExit = ExitCode::Generic;
    auto tracer = SelectTracer(rpiInfos, s, tracerExit);
    if (tracer.index < -1)
    {
      return AsInt(tracerExit);
    }
    // tracer.index == -1 here means no DICOM RPI sequence; CLI overrides must supply activity / half-life.
    // tracer.index >= 0 means a tracer was selected.

    const double injectedActivity = s.injectedActivityBq.has_value()
      ? s.injectedActivityBq.value()
      : tracer.info.totalDoseBq;

    const double halfLife = s.halfLifeS.has_value()
      ? s.halfLifeS.value()
      : tracer.info.halfLifeSeconds;

    if (!std::isfinite(injectedActivity))
    {
      MITK_ERROR << "Injected activity is unknown. Provide --injected-activity "
                    "or supply DICOM (0018,1074).";
      return AsInt(ExitCode::MissingDICOMProperty);
    }
    if (injectedActivity <= 0.0)
    {
      MITK_ERROR << "Injected activity must be a positive value (got "
                 << injectedActivity << " Bq). Provide --injected-activity "
                    "or check DICOM (0018,1074).";
      return AsInt(ExitCode::InvalidDICOMPropertyValue);
    }
    if (!std::isfinite(halfLife))
    {
      MITK_ERROR << "Half-life is unknown. Provide --half-life or --nuclide, "
                    "or supply DICOM (0018,1075).";
      return AsInt(ExitCode::MissingDICOMProperty);
    }
    if (halfLife <= 0.0)
    {
      MITK_ERROR << "Half-life must be a positive value (got "
                 << halfLife << " s). Provide --half-life or --nuclide, "
                    "or check DICOM (0018,1075).";
      return AsInt(ExitCode::InvalidDICOMPropertyValue);
    }

    // Patient measurements: always need body weight; LBM and BSA need
    // height; LBM also needs sex.
    mitk::SUVNormalizationInputs normInputs;
    normInputs.bodyWeightKg = s.bodyWeightKg.has_value()
      ? s.bodyWeightKg.value()
      : mitk::GetPatientsWeight(image.GetPointer());

    if (s.variant == mitk::SUVVariant::LBM || s.variant == mitk::SUVVariant::BSA)
    {
      normInputs.heightM = s.heightM.has_value()
        ? s.heightM.value()
        : mitk::GetPatientsHeight(image.GetPointer());
    }
    if (s.variant == mitk::SUVVariant::LBM)
    {
      normInputs.sex = s.sex.has_value()
        ? s.sex.value()
        : mitk::GetPatientsSex(image.GetPointer());
    }

    auto strategy = mitk::MakeSUVNormalizationStrategy(s.variant);
    const double scaleNumerator = strategy->ComputeScaleNumerator(normInputs);

    // ---- Decay times ---------------------------------------------------

    mitk::DecayCorrectionInfo decayInfo;
    if (s.decayTimeS.has_value())
    {
      // Uniform override. Build a synthetic per-(timestep, slice) map
      // with the same value everywhere. We deliberately do NOT consult
      // DICOM here so the override also works for non-DICOM inputs
      // (Example 5 in the user manual: NRRD without acquisition tags).
      // The user accepts that --decay-time is uniform-by-construction
      // and is therefore not a substitute for the per-slice handling
      // the NONE strategy applies when reading from DICOM.
      const auto timeSteps = image->GetTimeSteps();
      for (mitk::TimeStepType t = 0; t < timeSteps; ++t)
      {
        const auto* sliced = image->GetSlicedGeometry(t);
        const unsigned int slices = (nullptr != sliced) ? sliced->GetSlices() : 1u;
        for (unsigned int z = 0; z < slices; ++z)
        {
          decayInfo.decayTimes[t][z] = s.decayTimeS.value();
        }
      }
      decayInfo.strategy = mitk::DecayCorrectionStrategy::Manual;
    }
    else
    {
      decayInfo = mitk::DeduceDecayCorrection(image);
    }

    // ---- 4D-correct output assembly ------------------------------------

    const auto pixelType  = mitk::MakeScalarPixelType<double>();
    auto outputTimeGeom   = image->GetTimeGeometry()->Clone();

    auto output = mitk::Image::New();
    output->Initialize(pixelType, *outputTimeGeom,
                       /*channels*/ 1, image->GetTimeSteps());

    // Property carry-over so DICOM provenance (study/series UIDs, patient
    // info, time-resolved properties) survives. Time geometry is preserved
    // so per-timestep entries remain valid. NB: (0054,1001) Units = BQML
    // is carried over verbatim even though the SUV output unit is no
    // longer Bq/mL — see the user manual for the documented limitation.
    if (image->GetPropertyList())
    {
      for (const auto& [key, prop] : *image->GetPropertyList()->GetMap())
      {
        if (prop.IsNotNull())
        {
          output->SetProperty(key.c_str(), prop->Clone());
        }
      }
    }

    auto runStep = [&](mitk::TimeStepType t, const mitk::Image* stepIn)
    {
      const auto* slicedGeom = image->GetSlicedGeometry(t);
      const std::size_t expectedSlices =
        (nullptr != slicedGeom) ? slicedGeom->GetSlices() : 1u;
      const auto sliceDecay = DensifyDecayTimes(decayInfo.decayTimes[t], expectedSlices);
      ProcessTimeStep(stepIn, output, t,
                      injectedActivity, scaleNumerator, halfLife,
                      sliceDecay);
    };

    if (image->GetTimeSteps() == 1)
    {
      // 3D fast path
      runStep(0, image);
    }
    else
    {
      for (mitk::TimeStepType t = 0; t < image->GetTimeSteps(); ++t)
      {
        auto sel = mitk::ImageTimeSelector::New();
        sel->SetInput(image);
        sel->SetTimeNr(t);
        sel->UpdateLargestPossibleRegion();
        runStep(t, sel->GetOutput());
      }
    }

    // ---- Save -----------------------------------------------------------

    if (s.verbose) MITK_INFO << "Saving output: " << s.outFileName;
    mitk::IOUtil::Save(output, s.outFileName);

    if (s.verbose) MITK_INFO << "SUV computation finished.";
    return AsInt(ExitCode::Success);
  }
  catch (const mitk::AmbiguousDecayTimingException& e)
  {
    MITK_ERROR << "Ambiguous decay timing: " << e.GetDescription();
    return AsInt(ExitCode::AmbiguousDecayTiming);
  }
  catch (const mitk::InvalidDICOMPropertyValueException& e)
  {
    MITK_ERROR << "Invalid DICOM property value: " << e.GetDescription();
    return AsInt(ExitCode::InvalidDICOMPropertyValue);
  }
  catch (const mitk::MissingDICOMPropertyException& e)
  {
    MITK_ERROR << "Missing DICOM property: " << e.GetDescription();
    return AsInt(ExitCode::MissingDICOMProperty);
  }
  catch (const mitk::MissingSUVInputException& e)
  {
    MITK_ERROR << "Missing SUV input: " << e.GetDescription();
    return AsInt(ExitCode::MissingSUVInput);
  }
  catch (const mitk::SUVHelperException& e)
  {
    MITK_ERROR << "SUV helper error: " << e.GetDescription();
    return AsInt(ExitCode::Generic);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "MITK error: " << e.GetDescription();
    return AsInt(ExitCode::Generic);
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << e.what();
    return AsInt(ExitCode::Generic);
  }
  catch (...)
  {
    MITK_ERROR << "Unexpected error.";
    return AsInt(ExitCode::Generic);
  }
}
