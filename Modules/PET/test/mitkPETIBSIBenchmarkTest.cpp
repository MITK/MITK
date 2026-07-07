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
  /**
   * \brief Per-DRO entry in the IBSI regression manifest.
   *
   * Static, mirroring the upstream \c docs/DRO_list.csv at the pinned
   * commit. Only the directory id and a description are carried; the
   * target variant is always BW (see file header) and the expected
   * statistics are file-local constants.
   */
  struct BenchmarkCase
  {
    const char* id;
    const char* description;
  };

  constexpr double kExpectedMin    = 0.20;
  constexpr double kExpectedMedian = 1.00;
  constexpr double kExpectedMax    = 4.00;
  // Half-ulp of the 2-decimal precision the IBSI manual prescribes.
  constexpr double kPassTolerance  = 5e-3;

  // Tight grid-equivalence epsilons. Mirrors the upstream Python harness;
  // refuse to silently resample.
  constexpr double kCoordinateEpsMM = 1e-3;
  constexpr double kDirectionEps    = 1e-6;

  constexpr BenchmarkCase kBenchmarkCases[] = {
    // default + rescale-slope baseline (BQML pixel semantics).
    {"DRO_0_0",   "default Units=BQML, DC=START"},
    {"DRO_1_0",   "multiple Rescale Slope"},

    // Pixel unit semantics: pre-normalized SUV inputs (GML / CM2ML)
    // and Philips CNTS scaling factors. The filter classifies each via
    // (0054,1001) + (0054,1006) and renormalizes to SUVbw.
    {"DRO_2_0",   "Units=GML pre-normalized SUVbw"},
    {"DRO_2_1_0", "Units=GML pre-norm SUVlbm-James128, sex M"},
    {"DRO_2_1_1", "Units=GML pre-norm SUVlbm-James128, sex F"},
    {"DRO_2_1_2", "Units=GML pre-norm SUVlbm-James128, sex O (mean-of-M-and-F)"},
    {"DRO_2_2_0", "Units=GML pre-norm SUV-IBW, sex M"},
    {"DRO_2_2_1", "Units=GML pre-norm SUV-IBW, sex F"},
    {"DRO_2_2_2", "Units=GML pre-norm SUV-IBW, sex O (mean-of-M-and-F)"},
    {"DRO_2_3",   "Units=CM2ML pre-normalized SUVbsa"},
    {"DRO_2_4",   "Units=CNTS Philips SUV scale factor"},
    {"DRO_2_5",   "Units=CNTS Philips activity scale factor"},
    {"DRO_2_6_0", "Units=GML pre-norm SUVlbm-Janma, sex M"},
    {"DRO_2_6_1", "Units=GML pre-norm SUVlbm-Janma, sex F"},
    {"DRO_2_6_2", "Units=GML pre-norm SUVlbm-Janma, sex O (mean-of-M-and-F)"},

    // Dose / decay-correction handling.
    {"DRO_3_0",   "dose in MBq (auto-detect)"},
    {"DRO_3_1",   "DC=ADMIN"},
    {"DRO_3_2_0", "DC=START Step 3 (Siemens T_ave)"},
    {"DRO_3_2_1", "DC=START Step 4 (GE -dFrameRef)"},
    {"DRO_3_2_2", "DC=START Step 3 (Philips T_ave)"},
    {"DRO_3_3_0", "DC=START Step 1 (Siemens private datetime)"},
    {"DRO_3_3_1", "DC=START Step 1 (GE private datetime)"},
    {"DRO_3_4_0", "DC=NONE multi-AcqTime (Siemens)"},
    {"DRO_3_4_1", "DC=NONE multi-AcqTime (GE)"},
    {"DRO_3_4_2", "DC=NONE multi-AcqTime (Philips, CNTS)"},
    {"DRO_3_5_0", "DC=START Step 2 (Siemens AcqTime==SeriesTime)"},
    {"DRO_3_5_1", "DC=START Step 2 (GE AcqTime==SeriesTime)"},
    {"DRO_3_5_2", "DC=START Step 2 (Philips AcqTime==SeriesTime)"},

    // Radiopharmaceutical injection time tag variants.
    {"DRO_4_0",   "RP DateTime only ((0018,1078))"},
    {"DRO_4_1",   "RP Time only ((0018,1072))"},
    {"DRO_4_2",   "RP Time + 24h rollover correction"},

    // Non-FDG nuclide.
    {"DRO_5_0",   "Radionuclide Ga-68"},
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
        failures << "\n  [" << kase.id << "] (" << kase.description
                 << "): " << ex.what();
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
    const std::string ptDir   = DROPath(dataDir, kase.id, "PT");
    const std::string maskFile = JoinPath(DROPath(dataDir, kase.id, "mask"), "DRO_mask.nii.gz");

    // ---- Load PT series ------------------------------------------------
    mitk::PreferenceListReaderOptionsFunctor readerFunctor(
      { "MITK DICOM Reader v2 (autoselect)" }, { "" });
    auto petImage = mitk::IOUtil::Load<mitk::Image>(ptDir, &readerFunctor);
    if (petImage.IsNull())
    {
      throw std::runtime_error("PT series load failed at '" + ptDir + "'.");
    }

    // ---- Compute SUV ---------------------------------------------------
    //
    // Default invocation: target=BW, policy=Lenient. The filter's input
    // classifier reads (0054,1001) + (0054,1006) and renormalizes any
    // pre-normalized source variant to SUVbw automatically.
    auto filter = mitk::SUVImageFilter::New();
    filter->SetInput(petImage);
    filter->SetTargetVariant(mitk::SUVVariant::BW);
    filter->SetDICOMReadPolicy(mitk::DICOMReadPolicy::Lenient);
    filter->Update();
    const mitk::Image::Pointer suvImage = filter->GetOutput();
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
