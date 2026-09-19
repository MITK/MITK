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


#ifndef mitkSUVCalculationHelper_h
#define mitkSUVCalculationHelper_h

#include <limits>
#include <map>
#include <string>
#include <vector>

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkSlicedData.h>

#include <MitkPETExports.h>

namespace mitk
{
  class IPropertyProvider;

  /**
   * \brief Patient sex modeled as a closed set.
   *
   * Only the two values needed by the SUV normalization strategies are
   * accepted. Conversion from the open DICOM string (CS VR M / F / O / U)
   * happens once at the helper boundary in GetPatientsSex(); downstream
   * code never carries an open string sentinel.
   *
   * \sa GetPatientsSex
   */
  enum class Sex
  {
    Male,
    Female,
    /** DICOM (0010,0040) value 'O' (Other). Strategies that consume
     *  patient sex must define how this case is handled. The Janmahasatian
     *  LBM strategy follows the IBSI-SUV benchmark recommendation:
     *  the mean of the male- and female-specific scale numerators.
     *  DICOM 'U' (Unknown) has no benchmark-supported convention and is
     *  rejected at the helper boundary. */
    Other
  };

  /**
   * \brief Base class for failures originating from the SUV calculation helpers.
   *
   * Callers can catch this base type to react to any helper failure, or one of
   * the specialized derived classes to differentiate by error category.
   */
  class MITKPET_EXPORT SUVHelperException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(SUVHelperException, mitk::Exception);
  };

  /**
   * \brief A required DICOM property is missing on the input data.
   *
   * The exception message identifies the offending tag in human-readable form
   * (e.g. "(0054,1102) Decay Correction"). Callers that need to react to
   * "missing tag" specifically can catch this type rather than parsing message text.
   */
  class MITKPET_EXPORT MissingDICOMPropertyException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(MissingDICOMPropertyException, SUVHelperException);
  };

  /**
   * \brief A DICOM property is present but holds a value the helper does not understand.
   *
   * Raised, e.g., when (0054,1102) Decay Correction is something other than
   * ADMIN, START, or NONE. The exception message contains the tag and the
   * actual unrecognized value.
   */
  class MITKPET_EXPORT InvalidDICOMPropertyValueException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(InvalidDICOMPropertyValueException, SUVHelperException);
  };

  /**
   * \brief A multi-item Radiopharmaceutical Information Sequence (0054,0016)
   *        was supplied without an explicit tracer selection.
   *
   * Distinct from the generic \c InvalidDICOMPropertyValueException so a
   * caller (e.g. the CLI) can report this recoverable case separately: it
   * is resolved by selecting one item via the tracer-index override rather
   * than by fixing the input. Derives from
   * \c InvalidDICOMPropertyValueException so handlers that do not
   * distinguish the case still treat it as an invalid-value error.
   */
  class MITKPET_EXPORT MultiItemRadiopharmaceuticalSequenceException
    : public InvalidDICOMPropertyValueException
  {
  public:
    mitkExceptionClassMacro(MultiItemRadiopharmaceuticalSequenceException,
                            InvalidDICOMPropertyValueException);
  };

  /**
   * \brief Base class for failures triggered by a benchmark-recommended
   *        adaptation that the active policy refuses to perform.
   *
   * The IBSI-SUV benchmark catalogues a handful of recommendations that
   * help MITK accept real-world DICOM input (e.g. interpreting a
   * Radionuclide Total Dose value below 1e4 as MBq rather than Bq).
   * In \c DICOMReadPolicy::Lenient those recommendations are applied
   * silently-but-loudly (log at WARN). In \c DICOMReadPolicy::Strict
   * they are refused, and the helper throws an instance of this
   * exception (or one of its derived classes) so callers can decide
   * whether to abort or to escalate to the user.
   *
   * Catch this base type to map the entire category to one CLI exit
   * code or GUI message; catch a derived type when one specific
   * category needs a tailored reaction.
   *
   * \sa DICOMReadPolicy
   */
  class MITKPET_EXPORT BenchmarkAdaptationRequiredException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(BenchmarkAdaptationRequiredException, SUVHelperException);
  };

  /**
   * \brief Radionuclide Total Dose (0018,1074) is implausible as Bq and
   *        would have been reinterpreted as MBq, but the active policy
   *        is Strict.
   *
   * The DICOM standard prescribes Bq, but some scanners and post-processing
   * pipelines store the value in MBq. The IBSI-SUV recommendation
   * interprets values strictly between 0 and 1e4 as MBq and converts to Bq.
   * In \c DICOMReadPolicy::Strict that conversion is refused and this
   * exception is raised instead. The exception message contains the
   * offending value.
   */
  class MITKPET_EXPORT ImplausibleRadionuclideDoseException : public BenchmarkAdaptationRequiredException
  {
  public:
    mitkExceptionClassMacro(ImplausibleRadionuclideDoseException, BenchmarkAdaptationRequiredException);
  };

  /**
   * \brief Strategy DC=START would have been resolved via a vendor-specific
   *        empirical fallback (Steps 3 / 4), but the active policy is Strict.
   *
   * Steps 1 (vendor private decay datetime) and 2 (AcquisitionTime equals
   * SeriesTime) reflect spec-clean reference-time information. Steps 3 / 4
   * extend the chain with per-vendor empirical formulas (Siemens / Philips
   * \c AcquisitionTime + T_ave - FrameReferenceTime, GE
   * \c AcquisitionTime - FrameReferenceTime). The formulas are derived from
   * observed scanner behavior rather than a published DICOM-level
   * specification, so \c DICOMReadPolicy::Strict refuses to apply them and
   * raises this exception. Callers should either obtain spec-clean input
   * (Steps 1 / 2) or supply timing externally (e.g. CLI \c --decay-time).
   */
  class MITKPET_EXPORT VendorEmpiricalDecayFallbackRefusedException : public BenchmarkAdaptationRequiredException
  {
  public:
    mitkExceptionClassMacro(VendorEmpiricalDecayFallbackRefusedException, BenchmarkAdaptationRequiredException);
  };

  /**
   * \brief Tag (0054,1001) Units holds a value the SUV pipeline cannot
   *        interpret.
   *
   * Raised by SUVImageFilter when (0054,1001) is present but the value is
   * outside the documented set the pipeline supports
   * (BQML / GML / CM2ML / CNTS with Philips private factor). The
   * IBSI-SUV catalogue defines no recommended adaptation for unknown
   * Units values, so the exception is raised under both
   * DICOMReadPolicy::Lenient and DICOMReadPolicy::Strict. Callers that
   * want to force activity-concentration semantics regardless of the
   * tag (the legacy --ignore-units-check escape hatch) supply
   * SUVImageFilter::SetInputModelOverride explicitly.
   */
  class MITKPET_EXPORT UnsupportedPETUnitsException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(UnsupportedPETUnitsException, SUVHelperException);
  };

  /**
   * \brief (0054,1001) Units == "CNTS" and the manufacturer is Philips,
   *        but neither of the two Philips private scale factors is
   *        present.
   *
   * Philips CNTS PET data carries either a SUV-scale factor
   * (group 7053, element 0x00 under creator "Philips PET Private Group")
   * or an activity-scale factor (element 0x09). Without either factor
   * the pixel values cannot be converted into SUV, so the SUV pipeline
   * refuses the input under both policies.
   */
  class MITKPET_EXPORT MissingPhilipsPETScaleException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(MissingPhilipsPETScaleException, SUVHelperException);
  };

  /**
   * \brief Ambiguous patient sex would have been resolved as the IBSI
   *        mean of the male- and female-specific normalization, but
   *        the active policy is Strict.
   *
   * Sex-specific SUV variants (LBM_Janmahasatian, LBM_James128, IBW)
   * have publication formulas defined for Male and Female only. The
   * IBSI-SUV benchmark recommends, for ambiguous sex, using the mean
   * of the male- and female-specific scale numerators. "Ambiguous"
   * covers both \c Sex::Other and the case where no patient sex was
   * resolved at all (DICOM tag absent / unknown / no override). That
   * adaptation is applied automatically under
   * \c DICOMReadPolicy::Lenient (with a \c MITK_WARN). Under
   * \c DICOMReadPolicy::Strict it is refused and this exception is
   * raised; the caller must either supply an unambiguous patient sex
   * via override or relax the policy.
   *
   * The adaptation is invoked upstream of the
   * \c SUVNormalizationStrategy implementations, which themselves are
   * sex-closed and accept \c Sex::Male / \c Sex::Female only.
   */
  class MITKPET_EXPORT AmbiguousPatientSexAdaptationRefusedException : public BenchmarkAdaptationRequiredException
  {
  public:
    mitkExceptionClassMacro(AmbiguousPatientSexAdaptationRefusedException, BenchmarkAdaptationRequiredException);
  };

  /**
   * \brief Acquisition / radiopharmaceutical-injection timing cannot be reconciled.
   *
   * Raised in two situations:
   *   - Only (0018,1072) Radiopharmaceutical Start Time is present (no DateTime),
   *     the resulting decay duration is negative, and even after a single 24 h
   *     rollover correction it remains outside [0, 24 h].
   *   - (0018,1078) Radiopharmaceutical Start DateTime is present but yields a
   *     negative decay duration. The DateTime is unambiguous, so no silent
   *     rollover correction is applied; the situation is surfaced as an error.
   *
   * The exception message recommends re-exporting with (0018,1078) where applicable.
   */
  class MITKPET_EXPORT AmbiguousDecayTimingException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(AmbiguousDecayTimingException, SUVHelperException);
  };

  /**
   * \brief A user-supplied per-(timestep, slice) decay-time override map is
   *        not a complete and well-formed match for the input image geometry.
   *
   * Raised by \c SUVImageFilter::ConfigureFromProperties when the map
   * installed via \c SetDecayTimeOverrideMap either does not cover every
   * (timestep, slice) pair the input image owns, or contains entries for
   * (timestep, slice) coordinates that are out of range for the input.
   *
   * The filter does not silently fall back to DICOM-derived values for
   * missing entries: a partial map is an explicit caller bug. Callers that
   * want a DICOM-derived baseline plus a few overrides must seed the map
   * from \c GetEffectiveDecayCorrection() (after a Configure pass without
   * the override) and then mutate the entries they intend to override.
   *
   * The exception message names the first offending (timestep, slice).
   */
  class MITKPET_EXPORT InvalidDecayTimeMapException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(InvalidDecayTimeMapException, SUVHelperException);
  };

  /**
   * \brief A uniform decay-time override value is not a usable decay
   *        duration.
   *
   * Raised by \c SUVImageFilter::SetDecayTimeOverrideInSec for a
   * non-finite (NaN / infinite) value or a negative duration. Zero is
   * accepted: it reproduces ADMIN-style behaviour (residual decay factor
   * 2^0 = 1). A NaN value would otherwise propagate to an all-NaN output
   * and a negative value would scale the dose upward, both silently.
   */
  class MITKPET_EXPORT InvalidDecayTimeOverrideException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(InvalidDecayTimeOverrideException, SUVHelperException);
  };

  /**
   * \brief Raised when a uniform decay-time override and a per-(timestep,
   *        slice) decay-time override map are set simultaneously.
   *
   * The two override modes are mutually exclusive: each represents a
   * distinct caller intent (a single global value vs. a fully resolved
   * per-slice grid), so the filter refuses to silently drop one in favour
   * of the other. The caller must clear the active override explicitly
   * (\c ClearDecayTimeOverrideInSec or \c ClearDecayTimeOverrideMap)
   * before engaging the other mode.
   */
  class MITKPET_EXPORT ConflictingDecayTimeOverrideException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(ConflictingDecayTimeOverrideException, SUVHelperException);
  };

  /**
   * \brief Policy controlling whether benchmark-recommended adaptations
   *        of borderline / non-spec DICOM input are applied.
   *
   * The IBSI-SUV benchmark catalogues several recommendations that help
   * MITK accept real-world PET DICOM data without losing physical
   * meaning (unit reinterpretation, vendor-specific fallbacks, etc.).
   * Each recommendation has a clearly bounded trigger (an empirically
   * empty value range, a specific vendor private tag, …) and is
   * therefore safe to apply in routine processing — but validation,
   * regulatory, or strict-conformance contexts may want to refuse the
   * adaptation and surface the underlying input issue instead.
   *
   * \c Lenient   Apply the recommendation; emit \c MITK_WARN so the
   *              adaptation can be audited downstream. Default.
   * \c Strict    Refuse to adapt; raise a
   *              \c BenchmarkAdaptationRequiredException (or a
   *              category-specific subtype).
   *
   * The policy applies uniformly across all helpers that consult it.
   * Per-rule overrides are not supported: if you need finer control,
   * inspect the input upstream and pre-validate the offending tags.
   *
   * \sa BenchmarkAdaptationRequiredException
   */
  enum class DICOMReadPolicy
  {
    Lenient,
    Strict
  };

  /**
   * \brief Manufacturer family inferred from DICOM tag (0008,0070) Manufacturer.
   *
   * The IBSI-SUV-conformant decay-correction pipeline branches on
   * manufacturer family (Siemens / GE / Philips use different vendor
   * private datetime tags or different reference-time formulas). The
   * classification is substring-and-case-insensitive on the trimmed
   * (0008,0070) string: typical real-world values include "SIEMENS
   * Healthineers", "GE MEDICAL SYSTEMS", "GE HEALTHCARE", "Philips
   * Medical Systems".
   *
   * \c Other covers any unrecognized or empty value. The IBSI-SUV
   * benchmark spec is silent for these vendors; consuming strategies
   * must surface this honestly (e.g. raise
   * \c AmbiguousDecayTimingException) rather than silently extend a
   * vendor-specific formula to an input we cannot classify.
   */
  enum class ManufacturerFamily
  {
    Siemens,
    GE,
    Philips,
    Other
  };

  /**
   * \brief Classify the input's (0008,0070) Manufacturer into a family.
   *
   * Case-insensitive substring match against trimmed
   * (0008,0070) Manufacturer. Returns \c ManufacturerFamily::Other if
   * the tag is missing, empty, or matches none of the recognized vendors.
   *
   * \param[in] provider Source of DICOM properties.
   * \return The inferred manufacturer family.
   */
  ManufacturerFamily MITKPET_EXPORT GetManufacturerFamily(const mitk::IPropertyProvider* provider);

  /**
   * \brief Strategy describing how (or whether) the input pixel data has been
   *        decay-corrected by the scanner.
   *
   * Mirrors the values defined for DICOM tag (0054,1102) Decay Correction.
   * Determines which reference time the helper must use when computing the
   * residual decay correction needed for SUV.
   */
  enum class DecayCorrectionStrategy
  {
    /** Pixel data is already decay-corrected to the radiopharmaceutical
     *  administration time. No further correction is needed. */
    Admin,
    /** Pixel data is decay-corrected to a vendor-specific reference time, resolved by the DC=START fallback chain described above. */
    Start,
    /** Pixel data is not decay-corrected. The residual correction uses the
     *  per-slice acquisition date and time. */
    None,
    /** Decay time supplied manually (e.g., CLI override); not derived from DICOM. */
    Manual
  };

  /**
   * \brief Map storing the residual decay time in seconds per slice.
   *
   * The key is the slice index (z-index).
   */
  typedef std::map<mitk::SlicedData::IndexValueType, double> DecayTimeSliceMapType;

  /**
   * \brief Map storing per-time-step decay time slice maps.
   *
   * The outer key is the time step, the inner map stores decay times per slice index.
   */
  typedef std::map<mitk::TimeStepType, DecayTimeSliceMapType> DecayTimeMapType;

  /**
   * \brief Result of DeduceDecayCorrection().
   *
   * Bundles the detected DICOM decay-correction strategy together with the
   * decay-time map that the SUV functor should consume per (timestep, slice).
   *
   * The semantics of \c decayTimes depend on \c strategy:
   *   - Admin: every entry is 0.0; the SUV decay term reduces to 1 (2^0).
   *   - Start: every (timestep, slice) entry holds the same value
   *            (SeriesTime - InjectionDateTime in seconds).
   *   - None:  per-slice values (AcquisitionDateTime - InjectionDateTime).
   */
  struct MITKPET_EXPORT DecayCorrectionInfo
  {
    DecayCorrectionStrategy strategy = DecayCorrectionStrategy::None;
    DecayTimeMapType        decayTimes;
  };

  /**
   * \brief Per-item radiopharmaceutical metadata pulled from the
   *        Radiopharmaceutical Information Sequence (0054,0016).
   *
   * One instance corresponds to one item in the sequence. Fields not present
   * in the source DICOM are left at their default (NaN for numeric fields,
   * empty string for the name); callers should check for this rather than
   * assume completeness.
   */
  struct MITKPET_EXPORT RadiopharmaceuticalInfo
  {
    /** Radionuclide half-life in [s]. NaN if (0018,1075) was not present. */
    double      halfLifeSeconds = std::numeric_limits<double>::quiet_NaN();
    /** Radionuclide total (injected) dose in [Bq]. NaN if (0018,1074) was not present. */
    double      totalDoseBq     = std::numeric_limits<double>::quiet_NaN();
    /** Radionuclide name (code meaning). Empty if (0054,0300)/(0008,0104) was not present. */
    std::string name;
  };

  /**
   * \brief Read the Radiopharmaceutical Information Sequence as paired-by-item info.
   *
   * Enumerates the items of the Radiopharmaceutical Information Sequence
   * (0054,0016) by their concrete sequence-item index, and assembles one
   * RadiopharmaceuticalInfo per item. Within each item, the half-life
   * (0018,1075), total dose (0018,1074), and radionuclide code meaning
   * (0054,0300)[*](0008,0104) are read and bundled into the same struct.
   *
   * Index pairing is enforced: the i-th element of the returned vector
   * corresponds to the i-th item of the source sequence; fields missing from
   * an item come back as NaN / empty without affecting other items.
   *
   * Radionuclide Total Dose values strictly between 0 and 1e4 are
   * interpreted as the IBSI-SUV benchmark recommends: in
   * \c DICOMReadPolicy::Lenient the value is reinterpreted as MBq and
   * converted to Bq (factor 1e6) with a \c MITK_WARN announcing the
   * conversion. In \c DICOMReadPolicy::Strict the conversion is refused
   * and an \c ImplausibleRadionuclideDoseException is raised. Other
   * fields (half-life, name) are read verbatim regardless of policy.
   *
   * \param[in] provider Source of DICOM properties; typically the BaseData of a
   *            PET image.
   * \param[in] policy   Policy for handling values that the IBSI-SUV
   *                     benchmark recommends adapting. Defaults to
   *                     \c DICOMReadPolicy::Lenient (apply with WARN).
   * \return A vector of RadiopharmaceuticalInfo, ordered by sequence-item
   *         index. Empty if no Radiopharmaceutical Information Sequence is
   *         present or if \p provider is \c nullptr.
   * \throw ImplausibleRadionuclideDoseException if \p policy is
   *        \c DICOMReadPolicy::Strict and an item's (0018,1074) value is
   *        strictly between 0 and 1e4.
   *
   * \remark Multi-tracer datasets are surfaced honestly (more than one entry).
   *         Callers that only support one tracer should check
   *         \c result.size() and react accordingly.
   */
  std::vector<RadiopharmaceuticalInfo> MITKPET_EXPORT
  GetRadiopharmaceuticalInfos(const mitk::IPropertyProvider* provider,
                              DICOMReadPolicy policy = DICOMReadPolicy::Lenient);

  /**
   * \brief Get the patient's weight from DICOM properties.
   *
   * Extracts the patient weight from DICOM tag (0010,1030) stored in the
   * properties of the passed provider.
   *
   * \param[in] provider Source of DICOM properties.
   * \return The patient's weight in [kg].
   * \pre \p provider must point to a valid instance.
   * \pre \p provider must contain a DICOM patient weight property.
   * \throw MissingDICOMPropertyException if \p provider is \c nullptr or contains
   *        no patient-weight property.
   */
  double MITKPET_EXPORT GetPatientsWeight(const mitk::IPropertyProvider* provider);

  /**
   * \brief Get the patient's height (size) from DICOM properties.
   *
   * Extracts the patient height from DICOM tag (0010,1020) Patient Size,
   * stored in the properties of the passed provider.
   *
   * \param[in] provider Source of DICOM properties.
   * \return The patient's height in [m].
   * \pre \p provider must point to a valid instance.
   * \pre \p provider must contain a DICOM patient-size property.
   * \throw MissingDICOMPropertyException if \p provider is \c nullptr or
   *        contains no patient-size property.
   */
  double MITKPET_EXPORT GetPatientsHeight(const mitk::IPropertyProvider* provider);

  /**
   * \brief Get the patient's sex from DICOM properties as a typed enum.
   *
   * Reads DICOM tag (0010,0040) Patient Sex (CS VR). The string value is
   * matched case-insensitively after trimming surrounding whitespace.
   * The values M, F, and O (Other) are accepted; the policy applied to
   * \c Sex::Other is defined by the consuming normalization strategy and
   * follows the IBSI-SUV benchmark recommendation (mean of male- and
   * female-specific factors). DICOM's "U" (Unknown) and any other value
   * remain invalid so callers cannot silently fall through with an
   * under-specified input.
   *
   * \param[in] provider Source of DICOM properties.
   * \return The patient's sex as a \c Sex enum value.
   * \pre \p provider must point to a valid instance.
   * \pre \p provider must contain a DICOM patient-sex property.
   * \throw MissingDICOMPropertyException if \p provider is \c nullptr or
   *        contains no patient-sex property.
   * \throw InvalidDICOMPropertyValueException if (0010,0040) holds a
   *        value other than \c M, \c F, or \c O.
   */
  Sex MITKPET_EXPORT GetPatientsSex(const mitk::IPropertyProvider* provider);

  /**
   * \brief Detect the DICOM Decay Correction strategy of the input data.
   *
   * Reads DICOM tag (0054,1102) Decay Correction. The value is matched
   * case-insensitively after trimming surrounding whitespace.
   *
   * \param[in] provider Source of DICOM properties.
   * \return The detected DecayCorrectionStrategy.
   * \throw MissingDICOMPropertyException if (0054,1102) is not present.
   * \throw InvalidDICOMPropertyValueException if (0054,1102) is present but
   *        holds a value other than ADMIN, START, or NONE.
   *
   * \sa DeduceDecayCorrection
   */
  DecayCorrectionStrategy MITKPET_EXPORT GetDecayCorrectionStrategy(const mitk::IPropertyProvider* provider);

  /**
   * \brief Deduce the residual decay correction needed to compute SUV.
   *
   * Reads (0054,1102) Decay Correction and produces a DecayCorrectionInfo whose
   * \c decayTimes map can be plugged into the SUV functor unchanged. The
   * residual decay term then accounts only for what the scanner did NOT already
   * apply.
   *
   * \par DC = ADMIN
   * Pixel data is already decay-corrected to the administration time; the
   * helper fills the map with 0.0 for every (timestep, slice), so the SUV
   * decay term collapses to 2^0 = 1.
   *
   * \par DC = START
   * Pixel data is decay-corrected to a vendor-specific reference time. The
   * helper applies the IBSI-SUV-recommended fallback chain in priority order;
   * the first condition whose preconditions are met determines the result.
   * The numbered steps below define the chain; "Step N" anywhere in the PET
   * sources, tests, or commit messages refers back to this list.
   *
   * \anchor DCStartFallbackChain
   *  -# <b>Vendor private datetime tag.</b> Siemens: (0071,0x22) "SIEMENS
   *     MED PT" decay-correction datetime (lifted to property
   *     \c mitk.pet.SiemensDecayDateTime by \c BaseDICOMReaderService);
   *     GE: (0009,0x0D) "GEMS_PETD_01" scan datetime (lifted to
   *     \c mitk.pet.GEScanDateTime). Used as the uniform reference time
   *     when present and yielding a non-negative decay.
   *  -# <b>AcquisitionTime equals SeriesTime.</b> Manufacturer in
   *     {Siemens, GE, Philips} and (0008,0032) AcquisitionTime equals
   *     (0008,0031) SeriesTime in seconds at slice 0: use per-slice
   *     AcquisitionTime as the reference.
   *  -# <b>Siemens / Philips, vendor T_ave formula.</b> Manufacturer in
   *     {Siemens, Philips}, per-slice (0008,0032) AcquisitionTime,
   *     (0054,0x1300) FrameReferenceTime, and (0018,0x1242)
   *     ActualFrameDuration available: per-slice reference time =
   *     AcquisitionTime + T_ave - FrameReferenceTime, with T_ave the
   *     closed-form average count-rate time over the frame.
   *  -# <b>GE, vendor -ΔFrameRef formula.</b> Manufacturer = GE, same
   *     per-slice tags available: per-slice reference time =
   *     AcquisitionTime - FrameReferenceTime.
   *
   * Steps 3 and 4 are vendor-specific empirical formulas; they fire only
   * under \c DICOMReadPolicy::Lenient. Under \c DICOMReadPolicy::Strict
   * they are refused and the helper raises
   * \c VendorEmpiricalDecayFallbackRefusedException.
   *
   * If none of these applies — typically an "Other" manufacturer with no
   * private datetime tag and no per-slice frame timing — the helper raises
   * \c AmbiguousDecayTimingException rather than silently extending one of
   * the vendor-specific formulas to an input we cannot classify.
   *
   * \par DC = NONE
   * Pixel data is not decay-corrected. The voxel value is the count rate
   * averaged over the frame, which equals the instantaneous count rate at
   * AcquisitionTime + T_ave. The helper therefore computes per-slice decay =
   * (AcquisitionTime + T_ave) - InjectionDateTime, where T_ave is derived
   * from per-slice (0018,0x1242) ActualFrameDuration. Inputs that lack the
   * frame-duration tag raise \c MissingDICOMPropertyException; \c --decay-time
   * is the documented escape hatch for inputs whose timing must be supplied
   * out-of-band.
   *
   * The radiopharmaceutical injection time is read from
   * (0054,0016)[*](0018,1078) Radiopharmaceutical Start DateTime if present,
   * otherwise from (0054,0016)[*](0018,1072) Radiopharmaceutical Start Time
   * (in which case the acquisition date is used to assemble a complete
   * timestamp, with a 24 h rollover correction if the resulting decay would
   * be negative).
   *
   * \param[in] data            The input data. Must be a valid SlicedData
   *                            instance whose time geometry and DICOM
   *                            properties are populated.
   * \param[in] halfLifeSeconds The radionuclide half-life in seconds, used
   *                            for T_ave in DC=START Steps 3 and DC=NONE.
   *                            If \c NaN (default), the helper falls back
   *                            to item 0 of the Radiopharmaceutical
   *                            Information Sequence — convenient for tests
   *                            and single-tracer datasets, but multi-tracer
   *                            callers should pass the resolved value
   *                            explicitly.
   * \param[in] policy          Controls whether IBSI-SUV-recommended
   *                            empirical adaptations are applied. \c Lenient
   *                            (default) enables DC=START Steps 3 / 4 of
   *                            the \ref DCStartFallbackChain "DC=START
   *                            fallback chain"; \c Strict refuses them and
   *                            raises
   *                            \c VendorEmpiricalDecayFallbackRefusedException
   *                            so callers can surface the underlying input
   *                            ambiguity.
   * \return The strategy plus a fully populated decay-time map.
   * \throw mitk::Exception if \p data is \c nullptr.
   * \throw MissingDICOMPropertyException if a DICOM property required for the
   *        detected strategy is missing (including frame-timing tags for
   *        DC=NONE).
   * \throw InvalidDICOMPropertyValueException if (0054,1102) holds an
   *        unsupported value, or if a tag value cannot be parsed.
   * \throw AmbiguousDecayTimingException if the radiopharmaceutical injection
   *        time and the acquisition / series reference time cannot be
   *        reconciled into a non-negative decay duration within 24 h, or if
   *        none of the DC=START fallback conditions applies.
   * \throw VendorEmpiricalDecayFallbackRefusedException if \p policy is
   *        \c Strict and DC=START would have been resolved through
   *        Step 3 or Step 4 of the \ref DCStartFallbackChain "DC=START
   *        fallback chain" (vendor-specific empirical formula).
   *
   * \sa GetDecayCorrectionStrategy, GetManufacturerFamily,
   *     computeSUVbwScaleFactor
   */
  DecayCorrectionInfo MITKPET_EXPORT DeduceDecayCorrection(
    const mitk::SlicedData* data,
    double halfLifeSeconds = std::numeric_limits<double>::quiet_NaN(),
    DICOMReadPolicy policy = DICOMReadPolicy::Lenient);

}

#endif
