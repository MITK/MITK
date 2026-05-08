/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSUVImageFilter_h
#define mitkSUVImageFilter_h

#include <MitkPETExports.h>
#include <mitkImageToImageFilter.h>
#include <mitkSUVCalculationHelper.h>
#include <mitkSUVInputModel.h>
#include <mitkSUVNormalizationStrategy.h>
#include <optional>

namespace mitk
{
  class IPropertyProvider;

  /**
   * \brief Image-to-image filter that converts a PET image into a SUV image.
   *
   * Single entry point for SUV computation. Encapsulates parameter
   * resolution (CLI args / UI controls vs DICOM-derived defaults), the
   * decay-correction pipeline, the per-voxel SUV math, and the
   * variant-specific output normalization.
   *
   * \par Usage pattern
   *
   * \code
   * auto filter = mitk::SUVImageFilter::New();
   * filter->SetInput(petImage);
   * filter->SetTargetVariant(SUVVariant::BW);
   * if (overrideWeightKg) filter->SetPatientWeightInGram(*overrideWeightKg * 1000.0);
   * // ... other explicit overrides ...
   * filter->ConfigureFromProperties(petImage);   // mitk::Image is an IPropertyProvider
   * filter->Update();
   * auto suv = filter->GetOutput();
   * \endcode
   *
   * \par Override pattern
   *
   * Each user-overridable field is held internally as \c std::optional<T>.
   * An empty optional means "not set, derive from properties"; a populated
   * optional means "user supplied this value; do not overwrite."
   * itkSetMacro / itkGetConstMacro work directly: the macro setter takes
   * \c T, and \c std::optional<T> assigns and compares heterogeneously
   * against \c T (empty optional is unequal to any value, populated
   * optional compares by value, assigning a \c T engages the optional).
   *
   * Two accessor flavors per field:
   *   - \c GetXxx() returns the user-supplied override slot
   *     (\c std::optional<T>). Useful for UI round-tripping and
   *     "what did the caller pass?" diagnostics.
   *   - \c GetEffectiveXxx() returns the resolved value (\c T) (either
   *     provided by user or deduced from dicum) that the
   *     filter will use. \pre ConfigureFromProperties() (or Update())
   *     has been called successfully; otherwise the accessor throws
   *     \c mitk::Exception.
   *
   * \c ClearXxx() resets a field to derive-from-properties.
   *
   * \sa SUVNormalizationStrategy, SUVFunctorPolicy, DICOMReadPolicy,
   *     DeduceDecayCorrection
   */
  class MITKPET_EXPORT SUVImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(SUVImageFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);

    /**
     * \brief Set the target SUV variant.
     *
     * Always required, no override semantics. Default at construction
     * is \c SUVVariant::BW, matching the most common use case.
     */
    itkSetEnumMacro(TargetVariant, SUVVariant);
    itkGetConstMacro(TargetVariant, SUVVariant);

    /**
     * \brief Policy for IBSI-SUV-recommended adaptations of borderline /
     *        ambiguous DICOM input.
     *
     * Default \c DICOMReadPolicy::Lenient, mirroring
     * SUVCalculationHelper. Governs ConfigureFromProperties only; has no
     * effect on Update once the filter is configured.
     */
    itkSetEnumMacro(DICOMReadPolicy, DICOMReadPolicy);
    itkGetConstMacro(DICOMReadPolicy, DICOMReadPolicy);

    // ---- Override fields ---------------------------------------------------
    //
    // Set / Get use itk macros. Clear and GetEffective are hand-written.

    /** \brief Override DICOM (0010,1030) Patient Weight, in [g]. */
    itkSetMacro(PatientWeightInGram, double);
    itkGetConstMacro(PatientWeightInGram, std::optional<double>);
    void ClearPatientWeightInGram();
    double GetEffectivePatientWeightInGram() const;

    /** \brief Override DICOM (0010,1020) Patient Size, in [cm]. */
    itkSetMacro(PatientHeightInCm, double);
    itkGetConstMacro(PatientHeightInCm, std::optional<double>);
    void ClearPatientHeightInCm();
    double GetEffectivePatientHeightInCm() const;

    /** \brief Override DICOM (0010,0040) Patient Sex. */
    itkSetEnumMacro(PatientSex, Sex);
    itkGetConstMacro(PatientSex, std::optional<Sex>);
    void ClearPatientSex();
    Sex GetEffectivePatientSex() const;

    /** \brief Override DICOM (0018,1074) Radionuclide Total Dose, in [Bq]. */
    itkSetMacro(InjectedActivityInBq, double);
    itkGetConstMacro(InjectedActivityInBq, std::optional<double>);
    void ClearInjectedActivityInBq();
    double GetEffectiveInjectedActivityInBq() const;

    /** \brief Override DICOM (0018,1075) Radionuclide Half-Life, in [s]. */
    itkSetMacro(HalfLifeInSec, double);
    itkGetConstMacro(HalfLifeInSec, std::optional<double>);
    void ClearHalfLifeInSec();
    double GetEffectiveHalfLifeInSec() const;

    /**
     * \brief Override the per-(timestep, slice) decay duration, in [s].
     *
     * Mirrors the existing CLI \c --decay-time semantic: if set, the
     * value is applied uniformly to every voxel and the
     * DICOM-decay-correction pipeline is bypassed entirely.
     */
    itkSetMacro(DecayTimeOverrideInSec, double);
    itkGetConstMacro(DecayTimeOverrideInSec, std::optional<double>);
    void ClearDecayTimeOverrideInSec();
    DecayCorrectionInfo GetEffectiveDecayCorrection() const;

    /**
     * \brief Optional explicit selection for multi-item Radiopharmaceutical
     *        Information Sequence (0054,0016).
     *
     * If unset, sequences with exactly one item are accepted automatically
     * and multi-item sequences cause ConfigureFromProperties to throw.
     */
    itkSetMacro(TracerIndex, int);
    itkGetConstMacro(TracerIndex, std::optional<int>);
    void ClearTracerIndex();

    /**
     * \brief Override the pixel-semantics classification.
     *
     * If set, ConfigureFromProperties does not call ClassifyPETInput on
     * the property provider; the override is used directly. Replaces the
     * legacy CLI \c --ignore-units-check escape: callers that want to
     * force activity-concentration semantics regardless of (0054,1001)
     * supply
     * \c SetInputModelOverride({SUVPixelSemantics::ActivityConcentration, 1.0, ...}).
     */
    void SetInputModelOverride(SUVInputModel v);
    void ClearInputModelOverride();
    std::optional<SUVInputModel> GetInputModelOverride() const;
    SUVInputModel GetEffectiveInputModel() const;

    /**
     * \brief Resolve all unset fields from a property provider.
     *
     * Reads the standard PET DICOM tags (patient weight / height / sex,
     * radiopharmaceutical info, decay-correction strategy). Explicit
     * overrides set via the dedicated setters always take precedence
     * over property-derived values.
     *
     * \param[in] props The source of DICOM properties; typically the
     *                  filter's input image.
     * \pre \p props is not null.
     * \post Every \c GetEffective*() accessor required by the chosen
     *       target variant is safe to call. On exception, the filter
     *       returns to its pre-call state and effective accessors stay
     *       unsafe.
     *
     * \throw MissingDICOMPropertyException if a required property is
     *        absent and no override compensates for it.
     * \throw InvalidDICOMPropertyValueException if a property is present
     *        but holds an unparseable / unsupported value.
     * \throw AmbiguousDecayTimingException for unresolvable decay timing.
     * \throw BenchmarkAdaptationRequiredException (or a subclass) if
     *        DICOMReadPolicy is Strict and the input would have required
     *        an IBSI-SUV-recommended adaptation.
     */
    void ConfigureFromProperties(const IPropertyProvider *props);

  protected:
    SUVImageFilter();
    ~SUVImageFilter() override;

    void GenerateData() override;
    void GenerateOutputInformation() override;

  private:
    /** \brief Throw \c mitk::Exception with a "Configure first" message. */
    static void RequireConfigured(bool configured, const char *fieldName);

    /** \brief Compute the per-(timestep, slice) decay map from the override. */
    DecayCorrectionInfo BuildOverrideDecayInfo(const Image *image, double decayTime) const;

    /** \brief Process one 3D timestep through the SUV functor and write to \p dst. */
    void ProcessTimeStep(const Image *stepIn,
                         Image *dst,
                         TimeStepType dstStep,
                         double injectedActivity,
                         double scaleNumerator,
                         double halfLife,
                         const DecayTimeSliceMapType &sliceMap) const;

    SUVVariant m_TargetVariant{SUVVariant::BW};
    DICOMReadPolicy m_DICOMReadPolicy{DICOMReadPolicy::Lenient};

    // Override slots: empty == "not set by user, derive from properties".
    std::optional<double> m_PatientWeightInGram;
    std::optional<double> m_PatientHeightInCm;
    std::optional<Sex> m_PatientSex;
    std::optional<double> m_InjectedActivityInBq;
    std::optional<double> m_HalfLifeInSec;
    std::optional<double> m_DecayTimeOverrideInSec;
    std::optional<int> m_TracerIndex;
    std::optional<SUVInputModel> m_InputModelOverride;

    // Effective values resolved by ConfigureFromProperties().
    std::optional<double> m_EffectivePatientWeightInGram;
    std::optional<double> m_EffectivePatientHeightInCm;
    std::optional<Sex> m_EffectivePatientSex;
    std::optional<double> m_EffectiveInjectedActivityInBq;
    std::optional<double> m_EffectiveHalfLifeInSec;
    std::optional<DecayCorrectionInfo> m_EffectiveDecayCorrection;
    std::optional<SUVInputModel> m_EffectiveInputModel;
    bool m_Configured{false};
  };
} // namespace mitk

#endif
