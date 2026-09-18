/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/*============================================================================

This test consumes the IBSI-SUV digital reference object benchmark from
oncoray/suv_computation (https://github.com/oncoray/suv_computation),
licensed CC BY 4.0 by the IBSI / Image Biomarker Standardisation
Initiative (Vácha, Zwanenburg et al.). The DRO tree is fetched at
configure time and is not redistributed by MITK.

The benchmark phantoms are constructed such that any correct SUV
implementation produces (SUVmin, SUVmedian, SUVmax) = (0.20, 1.00, 4.00)
inside the shipped ROI mask, regardless of input pixel-unit semantics
(BQML / GML / CM2ML / CNTS) or source pre-normalization variant
(BW / LBM-Janma / LBM-James128 / IBW / BSA). The expected triple is
expressed in SUVbw; the SUVImageFilter automatically classifies the
input via (0054,1001) Units + (0054,1006) SUV Type and renormalizes
pre-normalized inputs to SUVbw. No per-DRO target-variant override is
needed -- every case runs with target = SUVVariant::BW under the
default Lenient policy.

The benchmark also ships DRO_error_* objects, for which SUV must *not*
be computed. Those are carried in the same manifest under
Expectation::Refusal: the filter has to throw, with the expected
exception type and a message naming the precondition that failed.
Asserting the type alone would be too weak -- five of them raise
MissingDICOMPropertyException -- so a regression in weight handling
could otherwise hide behind an unrelated missing-tag error.

A case may also declare which IBSI-SUV recommendations the filter is
required to apply, or required not to apply, via
SUVImageFilter::GetAdaptations(). The log is not reachable from a test,
so that record -- not MITK_WARN -- is what pins the behaviour.

A DRO may appear twice, once per DICOMReadPolicy. Under Strict, MITK
deliberately refuses inputs whose computation depends on reinterpreting
a borderline value, trading benchmark conformance for the guarantee
that nothing was silently adapted. Those refusals are a contract, so
they are pinned here alongside a Strict case that must still compute --
without the latter, a bug that refused everything under Strict would
read as success.

The manifest now covers all 58 upstream DROs. It stays strict-red with
no xfail machinery: a case may only enter once the implementation
satisfies it, and DRO_7_1_0 and DRO_7_3_1 are here as expected
refusals rather than expected values.

That distinction matters when reading a green run. Under the
benchmark's own scoring those two are failures -- it expects
0.20 / 1.00 / 4.00 from both -- so "58 under test" is not
"58 conformant". The conformance figure is 56/58; this suite asserts
that nothing regressed, including that the two MITK cannot compute
still refuse rather than quietly returning numbers.

ROI statistics are computed by direct masked iteration over the SUV
output and the mask image, *not* via mitk::ImageStatisticsCalculator.
The calculator silently resamples mismatched mask geometries via
ImageMaskGenerator, which biases the median through partial-volume
averaging at sphere boundaries; we instead assert tight grid
equivalence and iterate raw voxels (mirroring the upstream Python
benchmark harness, which uses np.median(suv[mask > 0]) on SimpleITK
arrays after a strict np.allclose grid check).

============================================================================*/

#include <mitkBaseGeometry.h>
#include <mitkException.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

#include <mitkSUVCalculationHelper.h>
#include <mitkSUVImageFilter.h>
#include <mitkSUVNormalizationStrategy.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkImage.h>
#include <itkImageRegionConstIterator.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  /** \brief What a manifest entry asserts about its DRO. */
  enum class Expectation
  {
    CanonicalTriple,  ///< (0.20, 1.00, 4.00) inside the shipped ROI mask.
    Refusal           ///< SUVImageFilter must throw; SUV is not computable.
  };

  /**
   * \brief Per-DRO entry in the IBSI regression manifest.
   *
   * Static, mirroring the upstream \c docs/DRO_list.csv at the pinned
   * commit. Only the directory id, a description and the expectation are
   * carried; the target variant is always BW (see file header) and the
   * expected statistics are file-local constants.
   */
  struct BenchmarkCase
  {
    const char* id;
    const char* description;
    Expectation expectation;
    /**
     * Refusal entries only: a substring the exception message must
     * contain. Five of the error DROs share MissingDICOMPropertyException,
     * so the type alone would let a regression in weight handling pass
     * behind an unrelated missing-tag error. Keep the substring minimal --
     * a tag number where there is one -- so rewording a message does not
     * churn the manifest.
     */
    const char* refusalEvidence = "";
    /**
     * Refusal entries only: GetNameOfClass() of the expected exception,
     * empty to accept any mitk::Exception. The type is what the CLI's
     * distinct exit codes rest on, so it is worth pinning wherever it
     * discriminates (notably 2 vs. 6, absent tag vs. invalid value).
     */
    const char* refusalExceptionClass = "";
    /**
     * Read policy the case runs under. A DRO may appear twice -- once
     * Lenient, once Strict -- as two independent entries, because Strict is
     * a deliberately non-conformant mode whose refusals are themselves a
     * contract worth pinning.
     */
    mitk::DICOMReadPolicy policy = mitk::DICOMReadPolicy::Lenient;
    /** Adaptation rules that must appear in SUVImageFilter::GetAdaptations(). */
    unsigned requiredAdaptations = 0u;
    /**
     * Adaptation rules that must NOT appear. Stated separately from
     * "required" rather than demanding an exactly-equal set: an emptiness
     * assertion would break the day an unrelated recommendation fires on the
     * same input, which is a false alarm, not a regression.
     */
    unsigned forbiddenAdaptations = 0u;
  };

  // A bitmask keeps the manifest entries constexpr. Bit positions mirror
  // SUVAdaptationRule, so the mapping needs no maintenance when the enum
  // grows.
  constexpr unsigned AdaptationBit(mitk::SUVAdaptationRule rule)
  {
    return 1u << static_cast<unsigned>(rule);
  }

  constexpr unsigned kAdaptVendorFallback =
    AdaptationBit(mitk::SUVAdaptationRule::VendorEmpiricalDecayFallback);
  constexpr unsigned kAdaptUnknownVendor =
    AdaptationBit(mitk::SUVAdaptationRule::UnrecognizedManufacturer);
  constexpr unsigned kAdaptAdminDateFromStartDateTime =
    AdaptationBit(mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartDateTime);
  constexpr unsigned kAdaptAdminDateFromStartTime =
    AdaptationBit(mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartTime);
  constexpr unsigned kAdaptAdminShiftedBackOneDay =
    AdaptationBit(mitk::SUVAdaptationRule::AdministrationTimeShiftedBackOneDay);
  // Any reconstruction of the administration datetime, whichever tag drove
  // it. Used to assert that a case did *not* need one.
  constexpr unsigned kAdaptAnyAdminSubstitution =
    kAdaptAdminDateFromStartDateTime | kAdaptAdminDateFromStartTime
    | kAdaptAdminShiftedBackOneDay;

  unsigned AdaptationMask(const std::vector<mitk::SUVAdaptation>& adaptations)
  {
    unsigned mask = 0u;
    for (const auto& adaptation : adaptations)
    {
      mask |= AdaptationBit(adaptation.rule);
    }
    return mask;
  }

  constexpr double kExpectedMin    = 0.20;
  constexpr double kExpectedMedian = 1.00;
  constexpr double kExpectedMax    = 4.00;
  // Half-ulp of the 2-decimal precision the IBSI manual prescribes.
  constexpr double kPassTolerance  = 5e-3;

  // Tight grid-equivalence epsilons. Mirrors the upstream Python harness;
  // refuse to silently resample.
  constexpr double kCoordinateEpsMM = 1e-3;
  constexpr double kDirectionEps    = 1e-6;

  constexpr Expectation kTriple  = Expectation::CanonicalTriple;
  constexpr Expectation kRefusal = Expectation::Refusal;

  constexpr mitk::DICOMReadPolicy kLenient = mitk::DICOMReadPolicy::Lenient;
  constexpr mitk::DICOMReadPolicy kStrict  = mitk::DICOMReadPolicy::Strict;

  constexpr BenchmarkCase kBenchmarkCases[] = {
    // default + rescale-slope baseline (BQML pixel semantics).
    {"DRO_0_0",   "default Units=BQML, DC=START", kTriple},
    {"DRO_1_0",   "multiple Rescale Slope", kTriple},

    // Pixel unit semantics: pre-normalized SUV inputs (GML / CM2ML)
    // and Philips CNTS scaling factors. The filter classifies each via
    // (0054,1001) + (0054,1006) and renormalizes to SUVbw.
    {"DRO_2_0",   "Units=GML pre-normalized SUVbw", kTriple},
    {"DRO_2_1_0", "Units=GML pre-norm SUVlbm-James128, sex M", kTriple},
    {"DRO_2_1_1", "Units=GML pre-norm SUVlbm-James128, sex F", kTriple},
    {"DRO_2_1_2", "Units=GML pre-norm SUVlbm-James128, sex O (mean-of-M-and-F)", kTriple},
    {"DRO_2_2_0", "Units=GML pre-norm SUV-IBW, sex M", kTriple},
    {"DRO_2_2_1", "Units=GML pre-norm SUV-IBW, sex F", kTriple},
    {"DRO_2_2_2", "Units=GML pre-norm SUV-IBW, sex O (mean-of-M-and-F)", kTriple},
    {"DRO_2_3",   "Units=CM2ML pre-normalized SUVbsa", kTriple},
    {"DRO_2_4",   "Units=CNTS Philips SUV scale factor", kTriple},
    {"DRO_2_5",   "Units=CNTS Philips activity scale factor", kTriple},
    {"DRO_2_6_0", "Units=GML pre-norm SUVlbm-Janma, sex M", kTriple},
    {"DRO_2_6_1", "Units=GML pre-norm SUVlbm-Janma, sex F", kTriple},
    {"DRO_2_6_2", "Units=GML pre-norm SUVlbm-Janma, sex O (mean-of-M-and-F)", kTriple},

    // Dose / decay-correction handling.
    {"DRO_3_0",   "dose in MBq (auto-detect)", kTriple},
    {"DRO_3_1",   "DC=ADMIN", kTriple},
    {"DRO_3_2_0", "DC=START Step 3 (Siemens T_ave)", kTriple},
    {"DRO_3_2_1", "DC=START Step 4 (GE -dFrameRef)", kTriple},
    {"DRO_3_2_2", "DC=START Step 3 (Philips T_ave)", kTriple},
    // Step 3 is the general fallback for every manufacturer except GE, so an
    // unclassifiable vendor resolves through it -- and says so twice: once
    // for the empirical formula, once for the vendor it could not verify.
    {"DRO_3_2_3", "DC=START Step 3 (unrecognized vendor)", kTriple, "", "",
     kLenient, kAdaptVendorFallback | kAdaptUnknownVendor, 0u},
    {"DRO_3_3_0", "DC=START Step 1 (Siemens private datetime)", kTriple},
    {"DRO_3_3_1", "DC=START Step 1 (GE private datetime)", kTriple},
    {"DRO_3_4_0", "DC=NONE multi-AcqTime (Siemens)", kTriple},
    {"DRO_3_4_1", "DC=NONE multi-AcqTime (GE)", kTriple},
    {"DRO_3_4_2", "DC=NONE multi-AcqTime (Philips, CNTS)", kTriple},
    // DC=NONE never consults the manufacturer, so an unrecognized vendor
    // must stay silent here. Asserted negatively on purpose: a warning that
    // fires everywhere is worth no more than none.
    {"DRO_3_4_3", "DC=NONE multi-AcqTime (unrecognized vendor)", kTriple, "", "",
     kLenient, 0u, kAdaptVendorFallback | kAdaptUnknownVendor},
    {"DRO_3_5_0", "DC=START Step 2 (Siemens AcqTime==SeriesTime)", kTriple},
    {"DRO_3_5_1", "DC=START Step 2 (GE AcqTime==SeriesTime)", kTriple},
    {"DRO_3_5_2", "DC=START Step 2 (Philips AcqTime==SeriesTime)", kTriple},
    // Step 2 carries no empirical content -- an AcquisitionTime that already
    // equals the SeriesTime names the reference instant outright -- so an
    // unrecognized vendor resolves with no adaptation at all.
    {"DRO_3_5_3", "DC=START Step 2 (unrecognized vendor)", kTriple, "", "",
     kLenient, 0u, kAdaptVendorFallback | kAdaptUnknownVendor},

    // Radiopharmaceutical administration time. These six must hold as a
    // set, because four of them reached the right answer before v3.0.1
    // through reasoning that has since been replaced: the fixed [0, 24 h]
    // acceptance window and its unconditional +24 h rollover. What keeps
    // them right now is the [-3600 s, 2 * T_half) window plus the
    // half-life-gated date reconstruction, so the branch each one takes is
    // asserted, not just its value.
    //
    // 4_0 and 4_3 are the controls: the only entries expected to need no
    // reconstruction at all, which is what proves the others are not
    // adapting spuriously.
    {"DRO_4_0",   "RP DateTime only ((0018,1078))", kTriple, "", "",
     kLenient, 0u, kAdaptAnyAdminSubstitution},
    {"DRO_4_1",   "RP Time only ((0018,1072)), date from reference", kTriple, "", "",
     kLenient, kAdaptAdminDateFromStartTime, kAdaptAdminShiftedBackOneDay},
    {"DRO_4_2",   "RP Time only, uptake spanning midnight", kTriple, "", "",
     kLenient, kAdaptAdminDateFromStartTime | kAdaptAdminShiftedBackOneDay, 0u},
    {"DRO_4_3",   "RP DateTime and Time, uptake spanning midnight", kTriple, "", "",
     kLenient, 0u, kAdaptAnyAdminSubstitution},
    // Acquisition and series dates are anonymized, so (0018,1078) yields a
    // ~65-year offset; its time of day is kept and the date rebuilt.
    {"DRO_4_4",   "RP DateTime with anonymized acquisition date", kTriple, "", "",
     kLenient, kAdaptAdminDateFromStartDateTime | kAdaptAdminShiftedBackOneDay, 0u},
    // Zr-89, three-day uptake. Inside 2 * T_half = 564552 s, so the stored
    // datetime is used verbatim -- the old flat 24 h ceiling rejected this.
    {"DRO_4_5",   "Zr-89, uptake beyond 24 h, within 2 * half-life", kTriple, "", "",
     kLenient, 0u, kAdaptAnyAdminSubstitution},

    // Non-FDG nuclide.
    {"DRO_5_0",   "Radionuclide Ga-68", kTriple},

    // Enhanced PET Image Storage. None of the classic PET attributes exist
    // in these objects: the unit lives in the Measurement Units Code
    // Sequence inside the functional groups, and (0018,9758) replaces
    // (0054,1102).
    {"DRO_7_0_0", "Enhanced PET, Bq/ml, DecayCorrected=YES", kTriple},
    {"DRO_7_2_0", "Enhanced PET, g/ml{SUVbw}", kTriple},
    // Frame times vary here too, exactly as in DRO_7_3_1 below, but
    // DecayCorrected=YES means the reference instant is the uniform
    // top-level (0018,9701) and the frame times do not enter the result.
    // This pair is the reason the frame-time guard is conditional.
    {"DRO_7_3_0", "Enhanced PET, reference from (0018,9701)", kTriple},

    // Error DROs: SUV must not be computed. The expected exception type
    // is pinned wherever it discriminates -- notably absent tag
    // (MissingDICOMProperty, CLI exit 2) against present-but-invalid
    // value (InvalidDICOMPropertyValue, CLI exit 6).
    {"DRO_error_2_0", "Units=BQML, Patient's Weight absent",
     kRefusal, "(0010,1030)", "MissingDICOMPropertyException"},
    {"DRO_error_2_1", "Units=BQML, Patient's Weight = 0",
     kRefusal, "Patient weight must be positive", "InvalidDICOMPropertyValueException"},
    {"DRO_error_2_2", "Units=GML SUV-IBW, Patient's Sex absent",
     kRefusal, "(0010,0040)", "MissingDICOMPropertyException"},
    {"DRO_error_2_3", "Units=GML SUV-IBW, Patient's Sex = X",
     kRefusal, "Patient Sex holds unsupported value", "InvalidDICOMPropertyValueException"},
    {"DRO_error_2_4", "Units=GML SUV-IBW, Patient's Size absent",
     kRefusal, "(0010,1020)", "MissingDICOMPropertyException"},
    {"DRO_error_2_5", "Units=GML SUV-IBW, Patient's Size = 0",
     kRefusal, "Patient height must be positive", "InvalidDICOMPropertyValueException"},
    {"DRO_error_2_6", "Units=CNTS, neither Philips scale factor present",
     kRefusal, "(7053,xx09)", "MissingPhilipsPETScaleException"},
    {"DRO_error_2_7", "Units=PROPCNTS (outside the supported set)",
     kRefusal, "'PROPCNTS'", "UnsupportedPETUnitsException"},
    {"DRO_error_3_0", "Radionuclide Total Dose absent",
     kRefusal, "(0018,1074)", "MissingDICOMPropertyException"},
    {"DRO_error_3_1", "Radionuclide Total Dose negative",
     kRefusal, "Injected activity must be a positive", "InvalidDICOMPropertyValueException"},
    {"DRO_error_3_2", "DC=START, Actual Frame Duration absent, non-GE",
     kRefusal, "fallback chain exhausted", "AmbiguousDecayTimingException"},
    {"DRO_error_4_0", "neither (0018,1078) nor (0018,1072) present",
     kRefusal, "neither (0018,1078)", "MissingDICOMPropertyException"},
    // This one refuses today only because a ~64-year offset falls outside
    // the fixed [0, 24 h] decay-duration window. That window is not the
    // reason IBSI-SUV v3.0.1 gives: it wants the refusal to come from the
    // half-life gate, since a date substitution is only permissible below
    // 41400 s and Zr-89 is far above it. Replacing the window is therefore
    // expected to turn this case red on the message, and the correct
    // response is to re-pin the evidence on the half-life gate -- never to
    // relax the entry. What must not change is that it keeps refusing.
    // Zr-89 with no (0018,1078). Rebuilding the date from the reference is
    // forbidden above a half-life of 41400 s, and that gate is now the only
    // thing standing between this input and a silently wrong SUV: before
    // v3.0.1 MITK computed it as a 0 s uptake and returned values 47 % low.
    {"DRO_error_4_1", "Zr-89, uptake > 24 h, (0018,1078) absent",
     kRefusal, "41400", "UnrecoverableAdministrationDateException"},
    // Refused today by the same gate. It used to refuse only by accident --
    // a ~65-year offset happened to fall outside the old [0, 24 h] window --
    // so replacing that window had to be watched closely here.
    {"DRO_error_4_2", "uptake > 24 h, (0018,1078) anonymized",
     kRefusal, "41400", "UnrecoverableAdministrationDateException"},
    {"DRO_error_5_0", "Radionuclide Half Life absent",
     kRefusal, "(0018,1075)", "MissingDICOMPropertyException"},

    // The two Enhanced PET objects MITK cannot yet compute. They are
    // carried as expected *refusals* rather than omitted, so all 58 DROs
    // are under test and the gap is stated in code rather than inferred
    // from an absence.
    //
    // Both need per-frame values that MITK's one-frame-per-file read model
    // cannot represent -- not a defect in the data. When the reader gains a
    // per-frame model these two will start failing here, which is the
    // intended signal: promote them to CanonicalTriple, do not delete them.
    //
    // Note that a green suite is therefore not a 58/58 conformance claim.
    // The benchmark expects 0.20 / 1.00 / 4.00 from both, so under its own
    // scoring a refusal is a failure and the conformance figure is 56/58.
    {"DRO_7_1_0", "Enhanced PET, per-frame Rescale Slope varies",
     kRefusal, "(0028,1053)", "EnhancedPETPerFrameVariationException"},
    {"DRO_7_3_1", "Enhanced PET, per-frame Frame Reference DateTime varies",
     kRefusal, "(0018,9151)", "EnhancedPETPerFrameVariationException"},

    // ---- Strict policy -------------------------------------------------
    //
    // --strict-dicom trades benchmark conformance for the guarantee that no
    // borderline value was reinterpreted, so under Strict MITK deliberately
    // refuses DROs the benchmark expects it to compute. That is a contract,
    // not a defect, and these rows pin it. Measured, not predicted: every
    // row below was confirmed by running the CLI with --strict-dicom.
    {"DRO_3_2_0", "DC=START Step 3 (Siemens), Strict refuses the empirical formula",
     kRefusal, "DICOMReadPolicy::Strict", "VendorEmpiricalDecayFallbackRefusedException",
     kStrict},
    {"DRO_3_2_2", "DC=START Step 3 (Philips), Strict refuses the empirical formula",
     kRefusal, "DICOMReadPolicy::Strict", "VendorEmpiricalDecayFallbackRefusedException",
     kStrict},
    {"DRO_3_2_3", "DC=START Step 3 (unrecognized vendor), Strict refuses",
     kRefusal, "DICOMReadPolicy::Strict", "VendorEmpiricalDecayFallbackRefusedException",
     kStrict},
    // The control for the three above: Step 2 is spec-clean, so Strict must
    // compute it. Without this row a bug that refuses everything under
    // Strict would look like success.
    {"DRO_3_5_3", "DC=START Step 2 (unrecognized vendor), Strict still computes",
     kTriple, "", "", kStrict, 0u, kAdaptVendorFallback | kAdaptUnknownVendor},
    // Rebuilding an administration date discards a value the input actually
    // carries, so Strict refuses it -- and with it three DROs the benchmark
    // expects to compute. That is the trade the mode exists to make.
    {"DRO_4_1", "RP Time only, Strict refuses the date reconstruction",
     kRefusal, "DICOMReadPolicy::Strict", "AdministrationDateSubstitutionRefusedException",
     kStrict},
    {"DRO_4_2", "RP Time spanning midnight, Strict refuses the reconstruction",
     kRefusal, "DICOMReadPolicy::Strict", "AdministrationDateSubstitutionRefusedException",
     kStrict},
    {"DRO_4_4", "anonymized acquisition date, Strict refuses the reconstruction",
     kRefusal, "DICOMReadPolicy::Strict", "AdministrationDateSubstitutionRefusedException",
     kStrict},
    // The controls: these need no reconstruction, so Strict must still
    // compute them. Without them a bug refusing every 4_x under Strict
    // would read as success.
    {"DRO_4_0", "RP DateTime only, Strict still computes", kTriple, "", "", kStrict},
    {"DRO_4_5", "Zr-89 within 2 * half-life, Strict still computes",
     kTriple, "", "", kStrict},
  };

  constexpr std::size_t kBenchmarkCaseCount = sizeof(kBenchmarkCases) / sizeof(kBenchmarkCases[0]);

  std::string JoinPath(const std::string& a, const std::string& b)
  {
    if (a.empty()) return b;
    return (a.back() == '/' || a.back() == '\\') ? (a + b) : (a + '/' + b);
  }

  std::string DROPath(const std::string& dataDir, const char* droId, const char* leaf)
  {
    return JoinPath(JoinPath(JoinPath(dataDir, "DRO"), droId), leaf);
  }

  std::string ToHex(unsigned value)
  {
    std::ostringstream os;
    os << std::hex << value;
    return os.str();
  }

  // Compute the median of a sorted double vector. Standard convention:
  // odd n -> middle element, even n -> mean of the two middle elements.
  double SortedMedian(const std::vector<double>& sorted)
  {
    const std::size_t n = sorted.size();
    if (0u == (n % 2u))
    {
      return 0.5 * (sorted[n / 2u - 1u] + sorted[n / 2u]);
    }
    return sorted[n / 2u];
  }
}  // namespace

class mitkPETIBSIBenchmarkTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPETIBSIBenchmarkTestSuite);
  MITK_TEST(RunIBSIBenchmark);
  CPPUNIT_TEST_SUITE_END();

public:

  /**
   * \brief Drive every IBSI DRO through SUVImageFilter and assert the
   *        canonical (0.20, 1.00, 4.00) SUV triple inside the shipped
   *        ROI mask.
   *
   * Iterates the manifest, accumulates per-case failures (rather than
   * stopping at the first), and emits a single CppUnit failure with a
   * compact per-case diagnostic block. Skips with exit 77 when the
   * data dir baked in at configure time (MITK_PET_IBSI_DATA_DIR cache
   * variable, see Modules/PET/test/CMakeLists.txt) is empty.
   */
  void RunIBSIBenchmark()
  {
#ifndef MITK_PET_IBSI_DATA_DIR
#define MITK_PET_IBSI_DATA_DIR ""
#endif
    const std::string dataDir(MITK_PET_IBSI_DATA_DIR);
    if (dataDir.empty())
    {
      MITK_INFO << "MITK_PET_IBSI_DATA_DIR was empty at configure time; skipping "
                   "the IBSI-SUV regression test. Configure with "
                   "MITK_PET_DOWNLOAD_IBSI_DATA=ON or set MITK_PET_IBSI_DATA_DIR "
                   "and rebuild to opt in.";
      std::exit(77);  // ctest SKIP_RETURN_CODE
    }

    std::ostringstream failures;
    std::size_t failureCount = 0;

    for (const auto& kase : kBenchmarkCases)
    {
      try
      {
        this->RunOneCase(dataDir, kase);
      }
      catch (const std::exception& ex)
      {
        ++failureCount;
        failures << "\n  [" << kase.id << "] ("
                 << (mitk::DICOMReadPolicy::Strict == kase.policy ? "Strict, " : "")
                 << kase.description << "): " << ex.what();
      }
    }

    if (0 != failureCount)
    {
      std::ostringstream summary;
      summary << failureCount << " of " << kBenchmarkCaseCount
              << " IBSI-SUV benchmark cases failed:" << failures.str();
      CPPUNIT_FAIL(summary.str());
    }
  }

private:

  static void RunOneCase(const std::string& dataDir, const BenchmarkCase& kase)
  {
    if (Expectation::Refusal == kase.expectation)
    {
      RunRefusalCase(dataDir, kase);
      return;
    }
    RunCanonicalTripleCase(dataDir, kase);
  }

  static mitk::Image::Pointer LoadPTSeries(const std::string& dataDir, const BenchmarkCase& kase)
  {
    const std::string ptDir = DROPath(dataDir, kase.id, "PT");
    mitk::PreferenceListReaderOptionsFunctor readerFunctor(
      { "MITK DICOM Reader v2 (autoselect)" }, { "" });
    auto petImage = mitk::IOUtil::Load<mitk::Image>(ptDir, &readerFunctor);
    if (petImage.IsNull())
    {
      throw std::runtime_error("PT series load failed at '" + ptDir + "'.");
    }
    return petImage;
  }

  /**
   * \brief Assert that the filter refuses this DRO, and for the stated
   *        reason.
   *
   * The series is read outside the guarded region on purpose: a read
   * failure is a broken test setup, not the refusal under test, and
   * accepting it as one would make the case pass for the wrong reason.
   */
  static void RunRefusalCase(const std::string& dataDir, const BenchmarkCase& kase)
  {
    auto petImage = LoadPTSeries(dataDir, kase);

    auto filter = mitk::SUVImageFilter::New();
    filter->SetInput(petImage);
    filter->SetTargetVariant(mitk::SUVVariant::BW);
    filter->SetDICOMReadPolicy(kase.policy);

    std::string actualClass;
    std::string actualMessage;
    try
    {
      filter->Update();
    }
    catch (const mitk::Exception& ex)
    {
      actualClass = ex.GetNameOfClass();
      const char* description = ex.GetDescription();
      actualMessage = (nullptr != description) ? description : "";
    }
    catch (const std::exception& ex)
    {
      actualClass = "<non-mitk std::exception>";
      actualMessage = ex.what();
    }

    if (actualClass.empty())
    {
      throw std::runtime_error("expected a refusal, but SUVImageFilter computed an output.");
    }

    const std::string expectedClass(kase.refusalExceptionClass);
    if (!expectedClass.empty() && actualClass != expectedClass)
    {
      throw std::runtime_error("refused as " + actualClass + ", expected " + expectedClass +
                               " (message: " + actualMessage + ").");
    }

    const std::string evidence(kase.refusalEvidence);
    if (!evidence.empty() && std::string::npos == actualMessage.find(evidence))
    {
      throw std::runtime_error("refused for the wrong reason: message does not contain '" +
                               evidence + "' (was: " + actualMessage + ").");
    }
  }

  static void RunCanonicalTripleCase(const std::string& dataDir, const BenchmarkCase& kase)
  {
    const std::string maskFile = JoinPath(DROPath(dataDir, kase.id, "mask"), "DRO_mask.nii.gz");

    auto petImage = LoadPTSeries(dataDir, kase);

    // ---- Compute SUV ---------------------------------------------------
    //
    // Default invocation: target=BW, policy=Lenient. The filter's input
    // classifier reads (0054,1001) + (0054,1006) and renormalizes any
    // pre-normalized source variant to SUVbw automatically.
    auto filter = mitk::SUVImageFilter::New();
    filter->SetInput(petImage);
    filter->SetTargetVariant(mitk::SUVVariant::BW);
    filter->SetDICOMReadPolicy(kase.policy);
    filter->Update();
    const mitk::Image::Pointer suvImage = filter->GetOutput();

    // The adaptation record, not the log, is what pins which IBSI-SUV
    // recommendations fired: MITK_WARN output is not reachable from a test.
    const unsigned actualAdaptations = AdaptationMask(filter->GetAdaptations());
    if (kase.requiredAdaptations != (actualAdaptations & kase.requiredAdaptations))
    {
      throw std::runtime_error("expected adaptation rules 0x" +
                               ToHex(kase.requiredAdaptations) + " but the filter recorded 0x" +
                               ToHex(actualAdaptations) + ".");
    }
    if (0u != (actualAdaptations & kase.forbiddenAdaptations))
    {
      throw std::runtime_error("filter applied adaptation rules 0x" +
                               ToHex(actualAdaptations & kase.forbiddenAdaptations) +
                               " that this input must not need.");
    }
    if (suvImage.IsNull())
    {
      throw std::runtime_error("SUVImageFilter produced a null output.");
    }

    // ---- Load NIfTI mask -----------------------------------------------
    auto maskImage = mitk::IOUtil::Load<mitk::Image>(maskFile);
    if (maskImage.IsNull())
    {
      throw std::runtime_error("Mask load failed at '" + maskFile + "'.");
    }

    // ---- Strict grid equivalence ---------------------------------------
    //
    // We refuse to silently resample. If the mask grid does not match
    // the SUV grid within tight tolerances, fail loud -- silent
    // resampling biases the median through partial-volume averaging.
    if (!mitk::Equal(*suvImage->GetGeometry(), *maskImage->GetGeometry(),
                     kCoordinateEpsMM, kDirectionEps, false))
    {
      throw std::runtime_error("SUV and mask geometries differ beyond tolerance "
                               "(coord=" + std::to_string(kCoordinateEpsMM) +
                               " mm, dir=" + std::to_string(kDirectionEps) + ").");
    }
    for (unsigned int d = 0; d < 3; ++d)
    {
      if (suvImage->GetDimension(d) != maskImage->GetDimension(d))
      {
        throw std::runtime_error("SUV and mask dimensions differ on axis " +
                                 std::to_string(d) + ".");
      }
    }

    // ---- Direct masked iteration (no resampling, no histogram) ---------
    using SUVImageT  = itk::Image<double, 3>;
    using MaskImageT = itk::Image<unsigned char, 3>;
    SUVImageT::Pointer  suvItk;
    MaskImageT::Pointer maskItk;
    mitk::CastToItkImage(suvImage,  suvItk);
    mitk::CastToItkImage(maskImage, maskItk);

    std::vector<double> values;
    values.reserve(static_cast<std::size_t>(
      suvItk->GetLargestPossibleRegion().GetNumberOfPixels()));

    itk::ImageRegionConstIterator<SUVImageT>  sIt(suvItk,  suvItk->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<MaskImageT> mIt(maskItk, maskItk->GetLargestPossibleRegion());
    for (sIt.GoToBegin(), mIt.GoToBegin();
         !sIt.IsAtEnd();
         ++sIt, ++mIt)
    {
      if (0u != mIt.Get())
      {
        values.push_back(sIt.Get());
      }
    }
    if (values.empty())
    {
      throw std::runtime_error("ROI mask is empty.");
    }

    std::sort(values.begin(), values.end());
    const double measMin = values.front();
    const double measMax = values.back();
    const double measMed = SortedMedian(values);

    // ---- Compare against canonical (0.20, 1.00, 4.00) ------------------
    std::ostringstream deviations;
    bool failed = false;
    if (std::abs(measMin - kExpectedMin) > kPassTolerance)
    {
      deviations << " min=" << measMin;
      failed = true;
    }
    if (std::abs(measMed - kExpectedMedian) > kPassTolerance)
    {
      deviations << " median=" << measMed;
      failed = true;
    }
    if (std::abs(measMax - kExpectedMax) > kPassTolerance)
    {
      deviations << " max=" << measMax;
      failed = true;
    }
    if (failed)
    {
      std::ostringstream msg;
      msg << "expected (min, median, max) = (" << kExpectedMin << ", "
          << kExpectedMedian << ", " << kExpectedMax << ") +-"
          << kPassTolerance << ", measured deviations:" << deviations.str();
      throw std::runtime_error(msg.str());
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPETIBSIBenchmark)
