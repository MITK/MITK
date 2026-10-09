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

#include <mitkDICOMTagPath.h>
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
   * \brief Patient's Weight would have been reinterpreted as grams, but
   *        the active policy is Strict.
   *
   * DICOM prescribes kilograms for (0010,1030), but exports storing
   * grams exist and are trivially recognizable: a value at or above
   * 1000 is not a plausible human weight in kilograms. The IBSI-SUV
   * recommendation reads such values as grams. Taken at face value
   * instead, the weight is 1000x too large and every SUV derived from
   * it 1000x too small -- an error large enough to be obvious, but only
   * to someone who looks.
   *
   * Under \c DICOMReadPolicy::Strict the reinterpretation is refused and
   * this exception raised instead.
   */
  class MITKPET_EXPORT ImplausiblePatientWeightException : public BenchmarkAdaptationRequiredException
  {
  public:
    mitkExceptionClassMacro(ImplausiblePatientWeightException, BenchmarkAdaptationRequiredException);
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
   * Raised when the DC=START fallback chain is exhausted -- no reference
   * time can be established from the available tags -- and, through its
   * subclass \c UnrecoverableAdministrationDateException, when the
   * administration date cannot be reconstructed because the half-life is
   * too long for the substitution to be safe.
   *
   * The exception message recommends re-exporting with (0018,1078) where applicable.
   */
  class MITKPET_EXPORT AmbiguousDecayTimingException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(AmbiguousDecayTimingException, SUVHelperException);
  };

  /**
   * \brief An administration *date* is required but cannot be recovered.
   *
   * When the offset between the decay-correction reference time and
   * (0018,1078) falls outside the plausible window, or when only
   * (0018,1072) is available, the IBSI-SUV recommendation reconstructs the
   * administration datetime from the reference date plus the stored time of
   * day. That substitution is only permissible below a half-life of
   * 41400 s: above it a date that is wrong by a whole day still yields a
   * plausible-looking SUV, so the error cannot be caught downstream. Such
   * input is refused under both policies -- unlike the substitution itself,
   * this is not an adaptation a caller may opt into.
   *
   * Derives from AmbiguousDecayTimingException so the refusal keeps the
   * decay-timing exit code.
   */
  class MITKPET_EXPORT UnrecoverableAdministrationDateException : public AmbiguousDecayTimingException
  {
  public:
    mitkExceptionClassMacro(UnrecoverableAdministrationDateException, AmbiguousDecayTimingException);
  };
  /**
   * \brief The frames of an Enhanced PET object name different units.
   *
   * The pipeline carries one unit per image, so a per-frame unit has no
   * representation. The object is refused under both policies, because
   * computing from one frame's unit would be silently wrong for the others.
   *
   * This is a MITK limitation rather than a defect in the input, and the
   * message says so.
   */
  class MITKPET_EXPORT EnhancedPETPerFrameVariationException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(EnhancedPETPerFrameVariationException, SUVHelperException);
  };

  /**
   * \brief The DICOM reader could not map the functional groups of a
   *        multi-frame Enhanced PET object to frames.
   *
   * No per-frame value (rescale, units, frame timing) reached MITK, and the
   * loaded pixels carry one frame's rescale throughout. The known causes
   * are a Per-Frame Functional Groups Sequence that does not carry one item
   * per frame, or none at all; the DICOM reader reports it in a warning for
   * the file.
   */
  class MITKPET_EXPORT EnhancedPETFramesUnresolvedException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(EnhancedPETFramesUnresolvedException, SUVHelperException);
  };

  /**
   * \brief No Real World Value Mapping of an Enhanced PET object describes
   *        the pixel values as loaded.
   *
   * The reader applies the Pixel Value Transformation per frame and leaves
   * the Real World Value Mapping to the consumer. The SUV pipeline does not
   * apply it either: it names the unit of the loaded values by the mapping
   * whose slope and intercept equal the applied transformation. When no
   * mapping does, the loaded values are in no unit the object declares, and
   * the input is refused rather than scaled by a guess. This is a MITK
   * limitation on a conformant object, not a defect in the input, and the
   * message says so.
   */
  class MITKPET_EXPORT EnhancedPETMappingNotAppliedException : public SUVHelperException
  {
  public:
    mitkExceptionClassMacro(EnhancedPETMappingNotAppliedException, SUVHelperException);
  };

  /**
   * \brief The administration date would have been substituted from the
   *        decay-correction reference datetime, but the policy is Strict.
   *
   * Replacing the stored administration date wholesale is a stronger
   * reinterpretation than the other benchmark recommendations: it discards
   * a value the input actually carries. Under
   * \c DICOMReadPolicy::Lenient it is applied with a \c MITK_WARN and
   * recorded as an \c SUVAdaptation; under \c Strict it is refused here.
   */
  class MITKPET_EXPORT AdministrationDateSubstitutionRefusedException : public BenchmarkAdaptationRequiredException
  {
  public:
    mitkExceptionClassMacro(AdministrationDateSubstitutionRefusedException, BenchmarkAdaptationRequiredException);
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
   * \brief Earliest decay duration the SUV pipeline accepts, in [s].
   *
   * Zero is the usual floor, but a dynamic scan may begin shortly before
   * administration; the IBSI-SUV recommendation tolerates up to an hour of
   * it (the manual reports real series acquired up to 166 s early). Below
   * this bound the decay term scales the dose upward rather than down,
   * which is never a legitimate reading of the input.
   *
   * The DICOM-derived path and the manual decay-time overrides share this
   * bound, so a user can always express what the pipeline computed.
   */
  constexpr double kEarliestDecayDurationSeconds = -3600.0;

  /**
   * \brief The closed set of IBSI-SUV-recommended reinterpretations the
   *        SUV pipeline may apply to a borderline input.
   *
   * Every value corresponds to one recommendation that \c DICOMReadPolicy
   * gates: under \c Lenient it is applied and recorded, under \c Strict it
   * raises the matching exception instead.
   */
  enum class SUVAdaptationRule
  {
    /** (0018,1074) below 1e4 read as MBq rather than Bq. */
    DoseReinterpretedAsMBq,
    /** DC=START resolved through a vendor-empirical reference-time formula. */
    VendorEmpiricalDecayFallback,
    /** (0008,0070) Manufacturer unrecognized; the general rule was applied. */
    UnrecognizedManufacturer,
    /** Ambiguous patient sex resolved as the mean of the male and female formulas. */
    AmbiguousPatientSexMeanOfMaleAndFemale,
    /**
     * The administration date was taken from the decay-correction reference
     * datetime and only the time component of (0018,1078) was kept.
     */
    AdministrationDateFromReferenceWithStartDateTime,
    /**
     * Same substitution, driven by (0018,1072), which carries no date of
     * its own.
     */
    AdministrationDateFromReferenceWithStartTime,
    /**
     * The substituted administration datetime landed after the reference,
     * so it was moved back one day ("injected last night, scanned this
     * morning").
     */
    AdministrationTimeShiftedBackOneDay,
    /** (0010,1030) Patient's Weight at or above 1000 read as grams. */
    WeightReinterpretedAsGrams
  };

  /**
   * \brief One reinterpretation that was applied while computing an SUV.
   *
   * A \c MITK_WARN records the same event for a human reading the log, but a
   * log is not available to a plugin, a pipeline or a Python caller. This
   * struct is the machine-readable half: it says which recommendation fired,
   * on which attribute, and what the value became.
   *
   * Under \c DICOMReadPolicy::Strict the list is always empty, because the
   * first adaptation throws instead of being applied. That emptiness is the
   * guarantee the mode exists to give, not an absence of information.
   *
   * \sa SUVImageFilter::GetAdaptations, DICOMReadPolicy
   */
  struct MITKPET_EXPORT SUVAdaptation
  {
    /** Which recommendation fired. */
    SUVAdaptationRule rule = SUVAdaptationRule::DoseReinterpretedAsMBq;
    /**
     * The DICOM tag concerned, e.g. "(0018,1074)". A tag nested in a sequence
     * is named by its full path, e.g. "(0054,0016)[1].(0018,1074)". Empty
     * when there is none. */
    std::string dicomTag;
    /** The value as stored in the input. */
    std::string originalValue;
    /** The value the computation actually used or, where the computation
     * used no single value, a description of the rule applied. */
    std::string usedValue;

    bool operator==(const SUVAdaptation&) const = default;
  };

  /**
   * \brief Stable identifier for an adaptation rule.
   *
   * The returned strings are part of the persisted record and of the
   * operator-facing summary, so they are fixed: renaming one silently
   * reinterprets every SUV image written before the change. The enum's
   * numeric values are deliberately not persisted for the same reason --
   * inserting a rule mid-enum would shift them all.
   *
   * \param[in] rule The rule to name.
   * \return The rule's stable identifier.
   */
  MITKPET_EXPORT const char* SUVAdaptationRuleToString(SUVAdaptationRule rule);

  /**
   * \brief Append an adaptation to \p adaptations, refusing under Strict.
   *
   * The single funnel through which the record is written. Every rule
   * has its own refusal that fires earlier under
   * \c DICOMReadPolicy::Strict, with an exception type specific enough
   * for a caller to act on; reaching the check here means a rule was
   * added without one. Refusing generically is the wrong diagnostic but
   * the right behaviour -- silently adapting under the policy that exists
   * to forbid adaptation is the one outcome that must not happen.
   *
   * The policy is checked before \p adaptations is, so a missing gate is
   * caught even when the caller is not collecting the record.
   *
   * The record holds distinct adaptations: an entry identical in every
   * field to one already recorded is not appended again, so a tag that
   * several computation steps consume in the same way counts once.
   *
   * \param[in,out] adaptations Record to append to; may be null.
   * \param[in] policy Active read policy.
   * \param[in] rule The rule that fired.
   * \param[in] dicomTag The tag concerned, or empty when there is none.
   * \param[in] originalValue The value as stored in the input.
   * \param[in] usedValue The value the computation used or, where it used no
   *            single value, a description of the rule applied.
   * \throws BenchmarkAdaptationRequiredException under Strict.
   */
  void MITKPET_EXPORT RecordAdaptation(std::vector<SUVAdaptation>* adaptations,
                                       DICOMReadPolicy policy,
                                       SUVAdaptationRule rule,
                                       const std::string& dicomTag,
                                       const std::string& originalValue,
                                       const std::string& usedValue);

  /**
   * \brief Render the adaptation record as an operator-facing summary.
   *
   * One line per adaptation, naming the rule, the tag it concerns and the
   * transition from the stored to the used value. Returns an empty string
   * for an empty record so a caller can print it unconditionally and stay
   * silent when nothing was adapted.
   *
   * \param[in] adaptations The record to render.
   * \return The summary, without a trailing newline, or empty.
   */
  std::string MITKPET_EXPORT FormatAdaptationSummary(const std::vector<SUVAdaptation>& adaptations);

  /**
   * \brief Serialize the adaptation record as a JSON array.
   *
   * The value of the \c mitk.pet.suv.adaptations property. Machine-readable
   * counterpart to the (0008,2111) Derivation Description, which is \c LO
   * and cannot carry the record itself.
   *
   * \param[in] adaptations The record to serialize.
   * \return A JSON array, "[]" for an empty record.
   */
  std::string MITKPET_EXPORT SerializeAdaptations(const std::vector<SUVAdaptation>& adaptations);

  /**
   * \brief Render the (0008,2111) Derivation Description for an SUV image.
   *
   * (0008,2111) is \c LO, 64 characters, so it carries the count rather
   * than the record. The count leads the string, which keeps it the first
   * thing lost to nothing and the last thing lost to truncation.
   *
   * \param[in] adaptations The record to describe.
   * \return A description of at most 64 characters.
   */
  std::string MITKPET_EXPORT FormatDerivationDescription(const std::vector<SUVAdaptation>& adaptations);

  /** \brief Property holding the SUV adaptation record as JSON. */
  constexpr const char* SUV_ADAPTATIONS_PROPERTY_NAME = "mitk.pet.suv.adaptations";

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
   * \c Other covers any unrecognized or empty value. Only the two
   * private-datetime rules (Step 1) and the GE reference-time formula
   * (Step 4) are genuinely vendor-specific; the remaining rules apply to
   * any manufacturer. An \c Other input therefore resolves through the
   * general rules rather than being refused, and the operator is warned
   * that an empirical formula was applied to an input whose scanner
   * behaviour could not be verified.
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

  /** \brief SOP Class UID of Enhanced PET Image Storage. */
  constexpr const char* ENHANCED_PET_SOP_CLASS_UID = "1.2.840.10008.5.1.4.1.1.130";

  /**
   * \brief Whether the input is an Enhanced PET Image Storage object.
   *
   * Decided solely by (0008,0016) SOP Class UID, so the Enhanced PET code
   * paths stay unreachable for every other input. Classic PET objects are
   * unaffected by anything this predicate gates.
   *
   * \param[in] provider Source of DICOM properties.
   * \return true when (0008,0016) equals the Enhanced PET SOP Class UID.
   */
  bool MITKPET_EXPORT IsEnhancedPETInput(const mitk::IPropertyProvider* provider);

  /**
   * \brief Findings about a Rescale Slope or Intercept the IBSI-SUV
   *        recommendations would object to.
   *
   * Diagnostic only: the values are never read back into the computation.
   * The reader has already applied them to the pixel buffer by the time
   * MITK sees the image, so re-applying them here would scale twice. What
   * this catches is input the operator should know about -- an absent
   * slope, which is silently treated as 1.0, or a non-zero intercept,
   * which shifts every voxel.
   *
   * Returned rather than only logged so a caller can put them in front of
   * the operator. They are not \c SUVAdaptation entries: nothing is
   * reinterpreted, and recording them would populate the adaptation record
   * under Strict, where the documented guarantee is that it is empty.
   *
   * Enhanced PET objects yield no findings: they carry no top-level
   * rescale, and their functional-group equivalents are validated by the
   * Enhanced PET classifier instead.
   *
   * \param[in] provider Source of DICOM properties.
   * \return One human-readable finding per objection; empty when the
   *         rescale values are unremarkable.
   */
  std::vector<std::string> MITKPET_EXPORT CheckRescalePlausibility(const mitk::IPropertyProvider* provider);

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
    /** Pixel data is decay-corrected to a scanner-chosen reference time,
     *  resolved as \c DeduceDecayCorrection documents. */
    Start,
    /** Pixel data is not decay-corrected. The residual correction uses each
     *  slice's measurement instant. */
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
   * The semantics of \c decayTimes depend on \c strategy; every entry is in
   * seconds, measured from the administration instant:
   *   - Admin:  every entry is 0.0; the SUV decay term reduces to 1 (2^0).
   *   - Start:  per (timestep, slice), up to the reference instant the slot's
   *             pixels are corrected to, as \c DeduceDecayCorrection resolves
   *             it. Slots may resolve to different instants.
   *   - None:   per (timestep, slice), up to the instant the slot's frame was
   *             measured (AcquisitionDateTime + T_ave, or an Enhanced PET
   *             frame's own reference instant).
   *   - Manual: every entry holds the supplied decay time, uniform or
   *             per (timestep, slice).
   */
  struct MITKPET_EXPORT DecayCorrectionInfo
  {
    DecayCorrectionStrategy strategy = DecayCorrectionStrategy::None;
    DecayTimeMapType        decayTimes;
    /**
     * Reinterpretations applied while deducing the decay correction. Empty
     * unless a DC=START fallback beyond the spec-clean Steps 1 and 2 was
     * needed, and always empty under \c DICOMReadPolicy::Strict.
     */
    std::vector<SUVAdaptation> adaptations;
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
    /**
     * Radionuclide total (injected) dose in [Bq], as the IBSI-SUV
     * recommendation interprets (0018,1074): the stored value times 1e6 when
     * \c totalDoseReinterpretedAsMBq, else the stored value. NaN if
     * (0018,1074) was not present. */
    double      totalDoseBq     = std::numeric_limits<double>::quiet_NaN();
    /** (0018,1074) as stored. Empty if absent. */
    std::string totalDoseStored;
    /**
     * Whether \c totalDoseBq is the stored value read as MBq per the
     * IBSI-SUV recommendation (stored value strictly between 0 and 1e4). */
    bool        totalDoseReinterpretedAsMBq = false;
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
   * The total dose is reported as the IBSI-SUV benchmark interprets it: a
   * stored value strictly between 0 and 1e4 is read as MBq, so
   * \c totalDoseBq holds it converted to Bq (factor 1e6) and
   * \c totalDoseReinterpretedAsMBq is set; \c totalDoseStored keeps the
   * stored string. The reader only reports. Applying the read policy and
   * recording the adaptation is the job of ApplyDosePlausibilityPolicy, for
   * the item the computation actually uses.
   *
   * \param[in] provider Source of DICOM properties; typically the BaseData
   *                     of a PET image.
   * \return A vector of RadiopharmaceuticalInfo, ordered by sequence-item
   *         index. Empty if no Radiopharmaceutical Information Sequence is
   *         present or if \p provider is \c nullptr.
   *
   * \remark Multi-tracer datasets are surfaced honestly (more than one entry).
   *         Callers that only support one tracer should check
   *         \c result.size() and react accordingly.
   *
   * \sa ApplyDosePlausibilityPolicy
   */
  std::vector<RadiopharmaceuticalInfo> MITKPET_EXPORT
  GetRadiopharmaceuticalInfos(const mitk::IPropertyProvider* provider);

  /**
   * \brief Apply the read policy to the dose of the Radiopharmaceutical
   *        Information Sequence item the computation uses.
   *
   * If \p info reports its dose as reinterpreted from MBq
   * (\c totalDoseReinterpretedAsMBq): under \c DICOMReadPolicy::Strict an
   * \c ImplausibleRadionuclideDoseException is thrown; under
   * \c DICOMReadPolicy::Lenient a \c MITK_WARN is logged and one
   * \c SUVAdaptationRule::DoseReinterpretedAsMBq entry is appended to
   * \p adaptations. Its \c dicomTag names the item's full path, e.g.
   * "(0054,0016)[1].(0018,1074)", \c originalValue is the stored string and
   * \c usedValue the Bq value as a Decimal String. Otherwise nothing happens.
   *
   * Only the item that feeds the computation should be passed: the doses of
   * other items, or a dose that an explicit activity overrides, leave the
   * result untouched.
   *
   * \param[in]     info        The item's info as read by GetRadiopharmaceuticalInfos.
   * \param[in]     item        Index of the item in the Radiopharmaceutical
   *                            Information Sequence; names the tag path in
   *                            the record and the messages.
   * \param[in]     policy      Active read policy.
   * \param[in,out] adaptations Appended to; never cleared.
   * \throw ImplausibleRadionuclideDoseException if \p policy is
   *        \c DICOMReadPolicy::Strict and the dose was reinterpreted.
   */
  void MITKPET_EXPORT ApplyDosePlausibilityPolicy(const RadiopharmaceuticalInfo& info,
                                                  int item,
                                                  DICOMReadPolicy policy,
                                                  std::vector<SUVAdaptation>& adaptations);

  /**
   * \brief Full path of (0018,1074) Radionuclide Total Dose inside item
   *        \p item of the Radiopharmaceutical Information Sequence.
   *
   * The path the adaptation record names and the SUV output writes the
   * reinterpreted dose to.
   *
   * \param[in] item Index of the item in (0054,0016).
   * \return The path, "(0054,0016)[item].(0018,1074)" in string form.
   */
  DICOMTagPath MITKPET_EXPORT RadiopharmaceuticalDoseTagPath(int item);

  /**
   * \brief Get the patient's weight from DICOM properties.
   *
   * Extracts the patient weight from DICOM tag (0010,1030) stored in the
   * properties of the passed provider. A value of 1000 or more is no
   * plausible weight in the kilograms DICOM prescribes; following the
   * IBSI-SUV recommendation, it is read as grams and recorded in
   * \p adaptations.
   *
   * \param[in]     provider    Source of DICOM properties.
   * \param[in]     policy      Active read policy.
   * \param[in,out] adaptations Appended to; never cleared.
   * \return The patient's weight in [kg].
   * \pre \p provider must point to a valid instance.
   * \pre \p provider must contain a DICOM patient weight property.
   * \throw MissingDICOMPropertyException if \p provider is \c nullptr or contains
   *        no patient-weight property.
   * \throw ImplausiblePatientWeightException if \p policy is
   *        \c DICOMReadPolicy::Strict and the weight would be read as grams.
   */
  double MITKPET_EXPORT GetPatientsWeight(const mitk::IPropertyProvider* provider,
                                         DICOMReadPolicy policy,
                                         std::vector<SUVAdaptation>& adaptations);

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
   * Pixel data is decay-corrected to a single, vendor-specific reference
   * time shared by every bed and frame of the series. The helper applies the
   * IBSI-SUV-recommended fallback chain in priority order. Step 1 decides for
   * the whole series: it applies only if every (timestep, slice) carries the
   * private datetime. Steps 2 to 4 are evaluated per (timestep, slice), and
   * the first step whose preconditions that slot meets determines its
   * reference time. A multi-bed or dynamic series therefore typically
   * resolves its first bed or frame through Step 2 and the others through
   * Step 3 or 4, all landing on the same instant. The numbered steps below
   * define the chain; "Step N" anywhere in the PET sources, tests, or commit
   * messages refers back to this list.
   *
   * \anchor DCStartFallbackChain
   *  -# <b>Vendor private datetime tag.</b> Siemens: (0071,0x22) "SIEMENS
   *     MED PT" decay-correction datetime (lifted to property
   *     \c mitk.pet.SiemensDecayDateTime by \c BaseDICOMReaderService);
   *     GE: (0009,0x0D) "GEMS_PETD_01" scan datetime (lifted to
   *     \c mitk.pet.GEScanDateTime). Applies when every slot carries a
   *     parseable private datetime; the resulting decay duration is then
   *     subject to the administration-time window like every other
   *     reference.
   *  -# <b>AcquisitionTime equals SeriesTime.</b> The slot's (0008,0032)
   *     AcquisitionTime equals (0008,0031) SeriesTime in seconds: use it as
   *     the slot's reference. Applies to every manufacturer and needs no
   *     frame timing.
   *  -# <b>General T_ave formula.</b> Any manufacturer other than GE,
   *     with per-slice (0008,0032) AcquisitionTime, (0054,0x1300)
   *     FrameReferenceTime and (0018,0x1242) ActualFrameDuration
   *     available: per-slice reference time = AcquisitionTime + T_ave -
   *     FrameReferenceTime, with T_ave the closed-form average count-rate
   *     time over the frame. This is the general fallback, not a
   *     Siemens/Philips special case.
   *  -# <b>GE, -FrameReferenceTime formula.</b> Manufacturer = GE, with
   *     per-slice AcquisitionTime and FrameReferenceTime: per-slice
   *     reference time = AcquisitionTime - FrameReferenceTime. Note that
   *     this rule needs no ActualFrameDuration -- it carries no T_ave
   *     term -- so a GE input that omits (0018,0x1242) still resolves.
   *
   * Steps 3 and 4 are empirical formulas derived from observed scanner
   * behaviour rather than from the DICOM specification; they fire only
   * under \c DICOMReadPolicy::Lenient. Under \c DICOMReadPolicy::Strict
   * a series is refused with
   * \c VendorEmpiricalDecayFallbackRefusedException as soon as one slot
   * needs either of them, so a multi-bed or dynamic series passes Strict
   * only if every slot is resolved by Step 1 or Step 2. Applying either
   * formula is recorded once as
   * \c SUVAdaptationRule::VendorEmpiricalDecayFallback, however many slots
   * used it, and applying it to an input whose manufacturer could not be
   * classified is additionally recorded as
   * \c SUVAdaptationRule::UnrecognizedManufacturer.
   *
   * If none of these applies to some slot -- typically no private datetime
   * tag and no frame timing -- the helper raises
   * \c AmbiguousDecayTimingException rather than inventing a reference
   * time. Resolving the other slots does not license a guess for that one.
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
   * \par Enhanced PET
   * An Enhanced PET object carries no (0054,1102); (0018,9758) Decay
   * Corrected decides instead: YES is reported as
   * \c DecayCorrectionStrategy::Start, NO as
   * \c DecayCorrectionStrategy::None. YES: the pixels are corrected to
   * (0018,9701) Decay Correction DateTime, used for every slice. NO: each
   * slice's reference is its frame's (0018,9151) Frame Reference DateTime,
   * or (0018,9074) Frame Acquisition DateTime plus T_ave from (0018,9220)
   * where a frame carries no reference instant. A multi-frame object whose
   * functional groups the reader did not map to frames is refused on both
   * branches with \c EnhancedPETFramesUnresolvedException.
   *
   * \par Administration time
   * Whichever rule above supplies the reference time, the decay duration
   * is measured from the radiopharmaceutical administration instant, which
   * is established in three branches keyed on the offset and the
   * half-life T:
   *   -# (0054,0016)[*](0018,1078) Radiopharmaceutical Start DateTime is
   *      present and the duration lies in [-3600 s, 2*T): use it as stored.
   *      The negative floor admits dynamic scans, where acquisition begins
   *      shortly before administration.
   *   -# (0018,1078) is present but the duration falls outside that window:
   *      keep its time of day and take the date from the reference instant.
   *   -# Only (0054,0016)[*](0018,1072) Radiopharmaceutical Start Time is
   *      present, which carries no date: same reconstruction.
   *
   * In branches 2 and 3 an administration instant that falls more than one
   * hour after the reference -- a duration below the same -3600 s floor as
   * branch 1 -- is moved back one day. Both branches are permitted only
   * while T is below 41400 s -- above it a date wrong by whole days still
   * yields a plausible SUV, so the input is refused instead. They are
   * benchmark adaptations: recorded and warned about under
   * \c DICOMReadPolicy::Lenient, refused under \c Strict.
   *
   * Datetimes carrying a UTC offset are normalized by it; one without an
   * offset is read as local to itself. Reference and administration
   * instants are always compared on the same basis.
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
   * \throw AmbiguousDecayTimingException if none of the DC=START fallback
   *        conditions applies to some (timestep, slice).
   * \throw UnrecoverableAdministrationDateException if the administration
   *        date would have to be reconstructed but the half-life is at or
   *        above 41400 s.
   * \throw AdministrationDateSubstitutionRefusedException if \p policy is
   *        \c Strict and that reconstruction would have been applied.
   * \throw VendorEmpiricalDecayFallbackRefusedException if \p policy is
   *        \c Strict and at least one (timestep, slice) would have been
   *        resolved through Step 3 or Step 4 of the
   *        \ref DCStartFallbackChain "DC=START fallback chain"
   *        (vendor-specific empirical formula).
   * \throw EnhancedPETFramesUnresolvedException if \p data is a
   *        multi-frame Enhanced PET object whose functional groups the reader
   *        did not map to frames, whichever (0018,9758) says.
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
