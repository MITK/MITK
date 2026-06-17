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
#include <optional>
#include <string>

// CTK includes
#include <mitkCommandLineParser.h>

// MITK Core / DICOM
#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkLog.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

// MITK PET
#include <mitkHalfLifeConstants.h>
#include <mitkSUVCalculationHelper.h>
#include <mitkSUVImageFilter.h>
#include <mitkSUVNormalizationStrategy.h>


namespace
{
  // Exit codes used by the CLI so calling scripts can branch on the
  // reason for failure rather than parse log messages.
  enum class ExitCode : int
  {
    Success                    = 0,
    Generic                    = 1,
    MissingDICOMProperty       = 2,
    AmbiguousDecayTiming       = 3,
    InvalidArguments           = 4,
    MultiTracerWithoutIndex    = 5,
    InvalidDICOMPropertyValue  = 6,
    MissingSUVInput            = 7,
    BenchmarkAdaptationRefused = 8,
    InputReadError             = 9,
    OutputWriteError           = 10,
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
    bool strictDicom         = false;
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
    if ("BW"           == v) return mitk::SUVVariant::BW;
    if ("LBM-JANMA"    == v) return mitk::SUVVariant::LBM_Janmahasatian;
    if ("LBM-JAMES128" == v) return mitk::SUVVariant::LBM_James128;
    if ("IBW"          == v) return mitk::SUVVariant::IBW;
    if ("BSA"          == v) return mitk::SUVVariant::BSA;
    return std::nullopt;
  }

  std::optional<mitk::Sex> ParseSex(const std::string& raw)
  {
    const std::string v = ToUpperAscii(TrimAsciiWhitespace(raw));
    if ("M" == v) return mitk::Sex::Male;
    if ("F" == v) return mitk::Sex::Female;
    if ("O" == v) return mitk::Sex::Other;
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
      "One of: bw (body weight, default), lbm-janma (lean body mass, "
      "Janmahasatian 2005, IBSI-SUV recommended), lbm-james128 (lean body "
      "mass, James 1976), ibw (ideal body weight, Sugawara 1999), "
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
      "Patient sex (M|F|O)",
      "Override DICOM (0010,0040) Patient Sex. Required for variant lbm. "
      "Value 'O' (Other) follows the IBSI-SUV benchmark convention: the "
      "mean of the male- and female-specific normalization factors.");
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
      "Force activity-concentration semantics (legacy --ignore-units-check). "
      "Equivalent to passing the input through the standard SUV formula as "
      "if Units = BQML, regardless of the actual (0054,1001) value. "
      "Bypasses both the input-units classification (no Philips private "
      "factor consulted) and any pre-normalized re-scale path.");
    parser.addArgument("strict-dicom", "", mitkCommandLineParser::Bool,
      "Strict DICOM input policy",
      "Refuse IBSI-SUV-recommended empirical adaptations of borderline / "
      "ambiguous DICOM input. Currently affects: (a) reinterpreting "
      "Radionuclide Total Dose (0018,1074) below 1e4 as MBq, and "
      "(b) DC=START vendor-specific decay-timing fallbacks (Siemens / "
      "Philips T_ave - FrameReferenceTime, GE -FrameReferenceTime). "
      "Without this flag the tool applies the recommendations and emits "
      "a WARN log entry. With this flag, supply unambiguous timing "
      "(vendor private datetime, AcquisitionTime == SeriesTime) or use "
      "--decay-time.");
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
        MITK_ERROR << "Invalid --variant value '" << raw
                   << "'. Expected one of bw, lbm-janma, lbm-james128, ibw, bsa.";
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
        MITK_ERROR << "Invalid --patient-sex value '" << raw << "'. Expected one of M, F, O.";
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
    if (parsed.count("strict-dicom"))          s.strictDicom         = us::any_cast<bool>(parsed.at("strict-dicom"));
    if (parsed.count("verbose"))               s.verbose             = us::any_cast<bool>(parsed.at("verbose"));

    return true;
  }

  // Validate (0008,0060) Modality == "PT" (case-insensitive, trimmed).
  bool ValidateModality(const mitk::Image* image, bool ignore)
  {
    const std::string raw = mitk::GetFirstDICOMValueAsString(image, mitk::DICOMTagPath(0x0008, 0x0060));
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

}


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  setupParser(parser);

  const auto parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.count("help"))
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

  // ---- Load ---------------------------------------------------------------
  //
  // Loaded in a dedicated try so an unreadable input is distinguishable
  // (InputReadError) from an internal computation error further down.
  mitk::Image::Pointer image;
  try
  {
    if (s.verbose) MITK_INFO << "Loading input: " << s.inFileName;
    mitk::PreferenceListReaderOptionsFunctor readerFunctor(
      { "MITK DICOM Reader v2 (autoselect)" }, { "" });
    image = mitk::IOUtil::Load<mitk::Image>(s.inFileName, &readerFunctor);
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Cannot load input image '" << s.inFileName << "': " << e.what();
    return AsInt(ExitCode::InputReadError);
  }
  if (image.IsNull())
  {
    MITK_ERROR << "Cannot load input image: " << s.inFileName;
    return AsInt(ExitCode::InputReadError);
  }

  try
  {
    // ---- Validate modality / units -------------------------------------

    if (!ValidateModality(image, s.ignoreModalityCheck))
    {
      return AsInt(ExitCode::InvalidArguments);
    }

    // ---- Configure filter ----------------------------------------------

    auto filter = mitk::SUVImageFilter::New();
    filter->SetInput(image);
    filter->SetTargetVariant(s.variant);
    filter->SetDICOMReadPolicy(s.strictDicom ? mitk::DICOMReadPolicy::Strict
                                             : mitk::DICOMReadPolicy::Lenient);
    if (s.injectedActivityBq) filter->SetInjectedActivityInBq(*s.injectedActivityBq);
    if (s.bodyWeightKg)       filter->SetPatientWeightInGram(*s.bodyWeightKg * 1000.0);
    if (s.heightM)            filter->SetPatientHeightInCm(*s.heightM * 100.0);
    if (s.sex)                filter->SetPatientSex(*s.sex);
    if (s.halfLifeS)          filter->SetHalfLifeInSec(*s.halfLifeS);
    if (s.decayTimeS)         filter->SetDecayTimeOverrideInSec(*s.decayTimeS);
    if (s.tracerIndex)        filter->SetTracerIndex(*s.tracerIndex);

    // Legacy --ignore-units-check escape: force activity-concentration
    // semantics regardless of (0054,1001).
    if (s.ignoreUnitsCheck)
    {
      mitk::SUVInputModel forced;
      forced.semantics     = mitk::SUVPixelSemantics::ActivityConcentration;
      forced.activityScale = 1.0;
      filter->SetInputModelOverride(forced);
      MITK_WARN << "--ignore-units-check is set; forcing activity-concentration "
                   "semantics. The resulting SUV will only be physically meaningful "
                   "if the input pixels really are in [Bq/mL].";
    }

    // Update() auto-configures from the input image when no explicit
    // ConfigureFromProperties has run, surfacing configuration errors with
    // the same exit-code mapping, so no separate call is needed here.
    filter->Update();

    auto output = filter->GetOutput();

    // ---- Save -----------------------------------------------------------
    //
    // Wrapped separately so an output-write failure is distinguishable
    // (OutputWriteError) from an internal computation error.
    try
    {
      if (s.verbose) MITK_INFO << "Saving output: " << s.outFileName;
      mitk::IOUtil::Save(output, s.outFileName);
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << "Cannot write output image '" << s.outFileName
                 << "': " << e.what();
      return AsInt(ExitCode::OutputWriteError);
    }

    if (s.verbose) MITK_INFO << "SUV computation finished.";
    return AsInt(ExitCode::Success);
  }
  catch (const mitk::AmbiguousDecayTimingException& e)
  {
    MITK_ERROR << "Ambiguous decay timing: " << e.GetDescription();
    return AsInt(ExitCode::AmbiguousDecayTiming);
  }
  catch (const mitk::BenchmarkAdaptationRequiredException& e)
  {
    MITK_ERROR << "Strict DICOM input policy refused a benchmark-recommended "
                  "adaptation: " << e.GetDescription();
    return AsInt(ExitCode::BenchmarkAdaptationRefused);
  }
  catch (const mitk::MultiItemRadiopharmaceuticalSequenceException& e)
  {
    MITK_ERROR << "Multi-item Radiopharmaceutical Information Sequence "
                  "without --tracer-index: " << e.GetDescription();
    return AsInt(ExitCode::MultiTracerWithoutIndex);
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
