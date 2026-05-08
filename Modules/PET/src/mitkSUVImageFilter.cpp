/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSUVImageFilter.h>

#include <cmath>
#include <limits>
#include <memory>
#include <sstream>

#include <itkImage.h>
#include <itkIndex.h>

#include <mitkExceptionMacro.h>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkPixelType.h>
#include <mitkProperties.h>

#include <itkIndexedUnaryFunctorImageFilter.h>
#include <itkUnaryFunctorImageFilter.h>
#include <mitkDICOMTagPath.h>
#include <mitkSUVFunctorPolicy.h>
#include <mitkTemporoSpatialStringProperty.h>

namespace
{
  using ImageT = itk::Image<double, 3>;

  // Per-voxel functor for the pre-normalized SUV path: a single scalar
  // multiply, no decay / dose math. The factor folds the input model's
  // prenorm scale and the source-to-target normalization ratio.
  class SUVRenormalizationFunctor
  {
  public:
    SUVRenormalizationFunctor() = default;
    explicit SUVRenormalizationFunctor(double factor) : m_Factor(factor) {}

    bool operator==(const SUVRenormalizationFunctor& other) const
    { return m_Factor == other.m_Factor; }
    bool operator!=(const SUVRenormalizationFunctor& other) const
    { return !(*this == other); }

    double operator()(const double& value) const { return value * m_Factor; }

  private:
    double m_Factor { 1.0 };
  };

  template <typename T, typename Fn>
  T ResolveValue(const std::optional<T>& override_,
                 Fn                       fromProperties)
  {
    return override_.has_value() ? override_.value() : fromProperties();
  }

  // True iff the variant's scale numerator depends on patient sex.
  bool IsSexSpecificVariant(mitk::SUVVariant v)
  {
    switch (v)
    {
      case mitk::SUVVariant::LBM_Janmahasatian:
      case mitk::SUVVariant::LBM_James128:
      case mitk::SUVVariant::IBW:
        return true;
      case mitk::SUVVariant::BW:
      case mitk::SUVVariant::BSA:
      default:
        return false;
    }
  }

  // True iff the variant's scale numerator needs the patient height.
  // BW is the only variant that does not consume height.
  bool NeedsHeight(mitk::SUVVariant v)
  {
    return v != mitk::SUVVariant::BW;
  }

  // Resolve the scale numerator under the ambiguous-sex policy gate.
  //
  // Sex-independent variants (BW, BSA) and sex-specific variants with a
  // concrete Sex::Male / Sex::Female pass straight through to the strategy.
  //
  // For sex-specific variants where sex is ambiguous (Sex::Other, or no
  // value at all because the upstream resolution left it unset), the
  // policy gate fires:
  //   - DICOMReadPolicy::Lenient: apply the IBSI mean-of-M-and-F (mean of
  //     strategy(Male) and strategy(Female)). Emits MITK_WARN.
  //   - DICOMReadPolicy::Strict: throw
  //     AmbiguousPatientSexAdaptationRefusedException.
  //
  // Treating "unset" and "Other" as the same kind of ambiguity matches
  // the IBSI-SUV recommendation (the adaptation targets ambiguous sex,
  // not a specific encoding) and prevents the gate from being silently
  // bypassed when an upstream path leaves the effective sex empty on a
  // sex-specific target variant.
  double ResolveScaleNumeratorUnderSexPolicy(
    const mitk::SUVNormalizationStrategy&  strategy,
    const mitk::SUVNormalizationInputs&    inputs,
    mitk::DICOMReadPolicy                  policy)
  {
    if (!IsSexSpecificVariant(strategy.Variant()))
    {
      return strategy.ComputeScaleNumerator(inputs);
    }

    const bool sexConcrete = inputs.sex.has_value() &&
                             mitk::Sex::Other != inputs.sex.value();
    if (sexConcrete)
    {
      return strategy.ComputeScaleNumerator(inputs);
    }

    const char* const trigger = inputs.sex.has_value()
      ? "= 'O' (Other)"
      : "is not available";

    if (mitk::DICOMReadPolicy::Strict == policy)
    {
      mitkThrowException(mitk::AmbiguousPatientSexAdaptationRefusedException)
        << "Patient sex (0010,0040) " << trigger << " and the chosen SUV "
           "variant is sex-specific. The IBSI mean-of-M-and-F adaptation "
           "is refused under DICOMReadPolicy::Strict. Provide an explicit "
           "patient-sex override or relax the policy to Lenient.";
    }

    MITK_WARN << "Patient sex (0010,0040) " << trigger << ". Applying the "
                 "IBSI-SUV recommended mean of male- and female-specific "
                 "scale numerators (DICOMReadPolicy::Lenient).";

    auto inputsCopy = inputs;
    inputsCopy.sex  = mitk::Sex::Male;
    const double scaleM = strategy.ComputeScaleNumerator(inputsCopy);
    inputsCopy.sex  = mitk::Sex::Female;
    const double scaleF = strategy.ComputeScaleNumerator(inputsCopy);
    return 0.5 * (scaleM + scaleF);
  }

  // Pick a single tracer from the Radiopharmaceutical Information Sequence.
  // Returns the index into \p infos, or -1 if the sequence is empty (caller
  // must then have overrides for activity / half-life). Throws on
  // multi-item sequences without an explicit selection.
  int SelectTracerIndex(const std::vector<mitk::RadiopharmaceuticalInfo>& infos,
                        const std::optional<int>&                         tracerIndex)
  {
    if (infos.empty()) return -1;

    if (1U == infos.size() && !tracerIndex.has_value()) return 0;

    if (tracerIndex.has_value())
    {
      const int idx = tracerIndex.value();
      if (idx < 0 || static_cast<std::size_t>(idx) >= infos.size())
      {
        mitkThrowException(mitk::InvalidDICOMPropertyValueException)
          << "Tracer index " << idx << " is out of range; sequence has "
          << infos.size() << " item(s).";
      }
      return idx;
    }

    std::ostringstream names;
    for (std::size_t i = 0; i < infos.size(); ++i)
    {
      if (i > 0) names << ", ";
      names << "[" << i << "] " << (infos[i].name.empty() ? "<unnamed>" : infos[i].name);
    }
    mitkThrowException(mitk::InvalidDICOMPropertyValueException)
      << "Input contains a multi-item Radiopharmaceutical Information "
         "Sequence (0054,0016). Set an explicit tracer index to select one. "
         "Items: " << names.str();
  }
}

mitk::SUVImageFilter::SUVImageFilter() = default;
mitk::SUVImageFilter::~SUVImageFilter() = default;

void mitk::SUVImageFilter::ClearPatientWeightInGram()
{
  if (m_PatientWeightInGram.has_value())
  {
    m_PatientWeightInGram.reset();
    this->Modified();
  }
}

void mitk::SUVImageFilter::ClearPatientHeightInCm()
{
  if (m_PatientHeightInCm.has_value())
  {
    m_PatientHeightInCm.reset();
    this->Modified();
  }
}

void mitk::SUVImageFilter::ClearPatientSex()
{
  if (m_PatientSex.has_value())
  {
    m_PatientSex.reset();
    this->Modified();
  }
}

void mitk::SUVImageFilter::ClearInjectedActivityInBq()
{
  if (m_InjectedActivityInBq.has_value())
  {
    m_InjectedActivityInBq.reset();
    this->Modified();
  }
}

void mitk::SUVImageFilter::ClearHalfLifeInSec()
{
  if (m_HalfLifeInSec.has_value())
  {
    m_HalfLifeInSec.reset();
    this->Modified();
  }
}

void mitk::SUVImageFilter::ClearDecayTimeOverrideInSec()
{
  if (m_DecayTimeOverrideInSec.has_value())
  {
    m_DecayTimeOverrideInSec.reset();
    this->Modified();
  }
}

void mitk::SUVImageFilter::ClearTracerIndex()
{
  if (m_TracerIndex.has_value())
  {
    m_TracerIndex.reset();
    this->Modified();
  }
}

void mitk::SUVImageFilter::SetInputModelOverride(SUVInputModel v)
{
  m_InputModelOverride = v;
  this->Modified();
}

void mitk::SUVImageFilter::ClearInputModelOverride()
{
  if (m_InputModelOverride.has_value())
  {
    m_InputModelOverride.reset();
    this->Modified();
  }
}

std::optional<mitk::SUVInputModel> mitk::SUVImageFilter::GetInputModelOverride() const
{
  return m_InputModelOverride;
}

mitk::SUVInputModel mitk::SUVImageFilter::GetEffectiveInputModel() const
{
  RequireConfigured(m_EffectiveInputModel.has_value(), "InputModel");
  return m_EffectiveInputModel.value();
}

void mitk::SUVImageFilter::RequireConfigured(bool configured, const char* fieldName)
{
  if (!configured)
  {
    mitkThrow() << "SUVImageFilter::GetEffective" << fieldName
                << "() called before ConfigureFromProperties() resolved this field. "
                   "Call ConfigureFromProperties() (or Update()) first.";
  }
}

double mitk::SUVImageFilter::GetEffectivePatientWeightInGram() const
{
  RequireConfigured(m_EffectivePatientWeightInGram.has_value(), "PatientWeightInGram");
  return m_EffectivePatientWeightInGram.value();
}

double mitk::SUVImageFilter::GetEffectivePatientHeightInCm() const
{
  RequireConfigured(m_EffectivePatientHeightInCm.has_value(), "PatientHeightInCm");
  return m_EffectivePatientHeightInCm.value();
}

mitk::Sex mitk::SUVImageFilter::GetEffectivePatientSex() const
{
  RequireConfigured(m_EffectivePatientSex.has_value(), "PatientSex");
  return m_EffectivePatientSex.value();
}

double mitk::SUVImageFilter::GetEffectiveInjectedActivityInBq() const
{
  RequireConfigured(m_EffectiveInjectedActivityInBq.has_value(), "InjectedActivityInBq");
  return m_EffectiveInjectedActivityInBq.value();
}

double mitk::SUVImageFilter::GetEffectiveHalfLifeInSec() const
{
  RequireConfigured(m_EffectiveHalfLifeInSec.has_value(), "HalfLifeInSec");
  return m_EffectiveHalfLifeInSec.value();
}

mitk::DecayCorrectionInfo mitk::SUVImageFilter::GetEffectiveDecayCorrection() const
{
  RequireConfigured(m_EffectiveDecayCorrection.has_value(), "DecayCorrection");
  return m_EffectiveDecayCorrection.value();
}

mitk::DecayCorrectionInfo mitk::SUVImageFilter::BuildOverrideDecayInfo(const Image* image,
                                                                      double       decayTime) const
{
  // Uniform override: build a synthetic per-(timestep, slice) map with the
  // same value everywhere. Mirrors the existing CLI --decay-time semantic.
  DecayCorrectionInfo info;
  info.strategy = DecayCorrectionStrategy::Manual;

  const auto timeSteps = image->GetTimeSteps();
  for (TimeStepType t = 0; t < timeSteps; ++t)
  {
    const auto* sliced = image->GetSlicedGeometry(t);
    const unsigned int slices = (nullptr != sliced) ? sliced->GetSlices() : 1U;
    for (unsigned int z = 0; z < slices; ++z)
    {
      info.decayTimes[t][z] = decayTime;
    }
  }
  return info;
}

void mitk::SUVImageFilter::ConfigureFromProperties(const IPropertyProvider* props)
{
  if (nullptr == props)
  {
    mitkThrow() << "SUVImageFilter::ConfigureFromProperties: provider is null.";
  }
  const Image* image = this->GetInput();
  if (nullptr == image)
  {
    mitkThrow() << "SUVImageFilter::ConfigureFromProperties: SetInput() must be called first.";
  }

  // Snapshot the previous effective state so we can roll back on exception.
  const auto prevWeight     = m_EffectivePatientWeightInGram;
  const auto prevHeight     = m_EffectivePatientHeightInCm;
  const auto prevSex        = m_EffectivePatientSex;
  const auto prevActivity   = m_EffectiveInjectedActivityInBq;
  const auto prevHalfLife   = m_EffectiveHalfLifeInSec;
  const auto prevDecay      = m_EffectiveDecayCorrection;
  const auto prevInputModel = m_EffectiveInputModel;
  const auto prevConf       = m_Configured;

  m_Configured = false;
  m_EffectivePatientWeightInGram.reset();
  m_EffectivePatientHeightInCm.reset();
  m_EffectivePatientSex.reset();
  m_EffectiveInjectedActivityInBq.reset();
  m_EffectiveHalfLifeInSec.reset();
  m_EffectiveDecayCorrection.reset();
  m_EffectiveInputModel.reset();

  try
  {
    // ---- Input pixel semantics ------------------------------------------

    m_EffectiveInputModel = m_InputModelOverride.has_value()
      ? m_InputModelOverride.value()
      : ClassifyPETInput(props, m_DICOMReadPolicy);

    // ---- Radiopharmaceutical info: tracer + activity + half-life ----------
    //
    // Only required for the activity-to-SUV path. Pre-normalized SUV
    // pixels skip these because the scanner has already absorbed the
    // dose / decay math.

    const bool needsRadioPharma =
      (SUVPixelSemantics::ActivityConcentration == m_EffectiveInputModel->semantics);

    if (needsRadioPharma)
    {
      auto rpiInfos      = GetRadiopharmaceuticalInfos(props, m_DICOMReadPolicy);
      const int tracerIx = SelectTracerIndex(rpiInfos, m_TracerIndex);
      const RadiopharmaceuticalInfo tracer = (tracerIx >= 0) ? rpiInfos[tracerIx]
                                                             : RadiopharmaceuticalInfo{};

      const double activity = ResolveValue<double>(m_InjectedActivityInBq, [&]() {
        return tracer.totalDoseBq;
      });
      if (!std::isfinite(activity))
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Injected activity is unknown. Provide an explicit override "
             "or supply DICOM (0018,1074).";
      }
      if (activity <= 0.0)
      {
        mitkThrowException(InvalidDICOMPropertyValueException)
          << "Injected activity must be a positive value (got "
          << activity << " Bq).";
      }
      m_EffectiveInjectedActivityInBq = activity;

      const double halfLife = ResolveValue<double>(m_HalfLifeInSec, [&]() {
        return tracer.halfLifeSeconds;
      });
      if (!std::isfinite(halfLife))
      {
        mitkThrowException(MissingDICOMPropertyException)
          << "Half-life is unknown. Provide an explicit override or supply "
             "DICOM (0018,1075).";
      }
      if (halfLife <= 0.0)
      {
        mitkThrowException(InvalidDICOMPropertyValueException)
          << "Half-life must be a positive value (got " << halfLife << " s).";
      }
      m_EffectiveHalfLifeInSec = halfLife;
    }

    // Body weight is consumed by every supported variant, either as the
    // target's scale numerator (BW) or as the BW denominator on the
    // renormalization path. Always required.
    const double weightG = m_PatientWeightInGram.has_value()
      ? m_PatientWeightInGram.value()
      : GetPatientsWeight(props) * 1000.0;
    if (!std::isfinite(weightG) || weightG <= 0.0)
    {
      mitkThrowException(InvalidDICOMPropertyValueException)
        << "Patient weight must be positive and finite (got " << weightG << " g).";
    }
    m_EffectivePatientWeightInGram = weightG;

    // Patient measurements: required by the target variant AND, for
    // pre-normalized inputs, by the source variant (cross-variant
    // renorm consumes patient measurements on both sides). The
    // input-model classification already ran above, so the source
    // variant is known here.
    const bool sourceNeedsRenormPatientData =
      (SUVPixelSemantics::PrenormalizedSUV == m_EffectiveInputModel->semantics);
    const SUVVariant sourceVariant = sourceNeedsRenormPatientData
      ? m_EffectiveInputModel->sourceVariant
      : m_TargetVariant;   // sentinel; not used when not needed

    const bool needsHeight = NeedsHeight(m_TargetVariant) ||
                             (sourceNeedsRenormPatientData && NeedsHeight(sourceVariant));
    if (needsHeight)
    {
      const double heightCm = m_PatientHeightInCm.has_value()
        ? m_PatientHeightInCm.value()
        : GetPatientsHeight(props) * 100.0;
      if (!std::isfinite(heightCm) || heightCm <= 0.0)
      {
        mitkThrowException(InvalidDICOMPropertyValueException)
          << "Patient height must be positive and finite (got " << heightCm << " cm).";
      }
      m_EffectivePatientHeightInCm = heightCm;
    }

    const bool needsSex = IsSexSpecificVariant(m_TargetVariant) ||
                          (sourceNeedsRenormPatientData &&
                           IsSexSpecificVariant(sourceVariant));
    if (needsSex)
    {
      m_EffectivePatientSex = m_PatientSex.has_value()
        ? m_PatientSex.value()
        : GetPatientsSex(props);
    }

    // ---- Decay correction -----------------------------------------------
    //
    // Only required for the activity-to-SUV path. Pre-normalized SUV
    // pixels skip decay correction entirely.

    if (needsRadioPharma)
    {
      if (m_DecayTimeOverrideInSec.has_value())
      {
        m_EffectiveDecayCorrection = BuildOverrideDecayInfo(image,
                                                            m_DecayTimeOverrideInSec.value());
      }
      else
      {
        m_EffectiveDecayCorrection = DeduceDecayCorrection(image,
                                                           m_EffectiveHalfLifeInSec.value(),
                                                           m_DICOMReadPolicy);
      }
    }

    m_Configured = true;
  }
  catch (...)
  {
    m_EffectivePatientWeightInGram  = prevWeight;
    m_EffectivePatientHeightInCm    = prevHeight;
    m_EffectivePatientSex           = prevSex;
    m_EffectiveInjectedActivityInBq = prevActivity;
    m_EffectiveHalfLifeInSec        = prevHalfLife;
    m_EffectiveDecayCorrection      = prevDecay;
    m_EffectiveInputModel           = prevInputModel;
    m_Configured                    = prevConf;
    throw;
  }
  this->Modified();
}

void mitk::SUVImageFilter::GenerateOutputInformation()
{
  Image::ConstPointer inputImage = this->GetInput();
  Image::Pointer      outputImage = this->GetOutput();

  if (inputImage.IsNull() || !inputImage->IsInitialized() ||
      nullptr == inputImage->GetTimeGeometry())
  {
    return;
  }

  // SUV pixel type is double regardless of input pixel type: the
  // formula's denominator can produce values that lose precision in a
  // typical float-PET range.
  outputImage->Initialize(MakeScalarPixelType<double>(),
                          *inputImage->GetTimeGeometry());

  if (inputImage->GetPropertyList())
  {
    outputImage->SetPropertyList(inputImage->GetPropertyList()->Clone());
  }
}

void mitk::SUVImageFilter::ProcessTimeStep(const Image*                 stepIn,
                                           Image*                       dst,
                                           TimeStepType                 dstStep,
                                           double                       injectedActivity,
                                           double                       scaleNumerator,
                                           double                       halfLife,
                                           const DecayTimeSliceMapType& sliceMap) const
{
  ImageT::Pointer itkIn;
  CastToItkImage(stepIn, itkIn);

  // Densify the slice map for O(1) lookup in the per-voxel hot loop.
  const auto* slicedGeom = stepIn->GetSlicedGeometry();
  const std::size_t expectedSlices =
    (nullptr != slicedGeom) ? slicedGeom->GetSlices() : 1U;
  std::vector<double> sliceDecay(expectedSlices,
                                 std::numeric_limits<double>::quiet_NaN());
  for (const auto& kv : sliceMap)
  {
    const auto idx = static_cast<std::size_t>(kv.first);
    if (idx < expectedSlices)
    {
      sliceDecay[idx] = kv.second;
    }
  }

  SUVFunctorPolicy functor(injectedActivity, scaleNumerator, halfLife);
  functor.SetDecayTimeFunctor(
    [&sliceDecay](const itk::Index<3>& idx) {
      const auto z = static_cast<std::size_t>(idx[2]);
      return (z < sliceDecay.size()) ? sliceDecay[z]
                                     : std::numeric_limits<double>::quiet_NaN();
    });

  if (!functor.IsConfigured())
  {
    mitkThrow() << "SUV functor is not fully configured (NaN scalar parameter).";
  }

  using FilterT = itk::IndexedUnaryFunctorImageFilter<ImageT, ImageT, SUVFunctorPolicy>;
  auto filter = FilterT::New();
  filter->SetFunctor(functor);
  filter->SetInput(itkIn);
  filter->Update();

  auto suvSlab = ImportItkImage(filter->GetOutput());
  ImageReadAccessor acc(suvSlab);
  dst->SetVolume(acc.GetData(), dstStep);
}

namespace
{
  // Pre-normalized path: one scalar multiply per voxel.
  void ProcessTimeStepRenormalized(const mitk::Image*  stepIn,
                                   mitk::Image*        dst,
                                   mitk::TimeStepType  dstStep,
                                   double              factor)
  {
    ImageT::Pointer itkIn;
    mitk::CastToItkImage(stepIn, itkIn);

    using FilterT = itk::UnaryFunctorImageFilter<ImageT, ImageT, SUVRenormalizationFunctor>;
    auto filter = FilterT::New();
    filter->SetFunctor(SUVRenormalizationFunctor(factor));
    filter->SetInput(itkIn);
    filter->Update();

    auto suvSlab = mitk::ImportItkImage(filter->GetOutput());
    mitk::ImageReadAccessor acc(suvSlab);
    dst->SetVolume(acc.GetData(), dstStep);
  }

  const char* OutputUnitsValue(mitk::SUVVariant target)
  {
    switch (target)
    {
      case mitk::SUVVariant::BW:
      case mitk::SUVVariant::LBM_Janmahasatian:
      case mitk::SUVVariant::LBM_James128:
      case mitk::SUVVariant::IBW:
        return "GML";
      case mitk::SUVVariant::BSA:
        return "CM2ML";
    }
    return nullptr;
  }

  // Both LBM variants share the standard SUV Type code "LBM"; the
  // specific formula is recorded externally (e.g. via filename or in a
  // study log).
  const char* OutputSUVTypeValue(mitk::SUVVariant target)
  {
    switch (target)
    {
      case mitk::SUVVariant::BW:                return "BW";
      case mitk::SUVVariant::LBM_Janmahasatian: return "LBM";
      case mitk::SUVVariant::LBM_James128:      return "LBM";
      case mitk::SUVVariant::IBW:               return "IBW";
      case mitk::SUVVariant::BSA:               return "BSA";
    }
    return nullptr;
  }

  // Set / overwrite the output's PET-image-module DICOM tags so they
  // describe the produced SUV image rather than the (now-stale) input.
  // RescaleIntercept and RescaleSlope are reset because SUV is already
  // in physical units; the filter applies no further rescale.
  void ApplyOutputTagPolicy(mitk::Image* output, mitk::SUVVariant target)
  {
    auto setStr = [output](const mitk::DICOMTagPath& path, const char* value) {
      if (nullptr == value) return;
      const std::string key = mitk::DICOMTagPathToPropertyName(path);
      output->SetProperty(key.c_str(),
                          mitk::TemporoSpatialStringProperty::New(value));
    };

    setStr(mitk::DICOMTagPath(0x0054, 0x1001), OutputUnitsValue(target));
    setStr(mitk::DICOMTagPath(0x0054, 0x1006), OutputSUVTypeValue(target));
    setStr(mitk::DICOMTagPath(0x0028, 0x1052), "0.0");
    setStr(mitk::DICOMTagPath(0x0028, 0x1053), "1.0");
  }
}

void mitk::SUVImageFilter::GenerateData()
{
  Image::ConstPointer inputImage  = this->GetInput();
  Image::Pointer      outputImage = this->GetOutput();

  if (inputImage.IsNull())
  {
    mitkThrow() << "SUVImageFilter::GenerateData: no input.";
  }
  if (outputImage.IsNull() || !outputImage->IsInitialized())
  {
    mitkThrow() << "SUVImageFilter::GenerateData: output not initialized "
                   "(GenerateOutputInformation must have run).";
  }

  // ConfigureFromProperties is the explicit, externally-callable resolution
  // step. We call it here only if the user has not done so already, using
  // the input image as the property source.
  if (!m_Configured)
  {
    this->ConfigureFromProperties(inputImage);
  }

  SUVNormalizationInputs normInputs;
  normInputs.bodyWeightKg = GetEffectivePatientWeightInGram() / 1000.0;
  if (m_EffectivePatientHeightInCm.has_value())
  {
    normInputs.heightM = m_EffectivePatientHeightInCm.value() / 100.0;
  }
  if (m_EffectivePatientSex.has_value())
  {
    normInputs.sex = m_EffectivePatientSex.value();
  }

  auto targetStrategy = MakeSUVNormalizationStrategy(m_TargetVariant);
  const double scaleNumeratorTarget =
    ResolveScaleNumeratorUnderSexPolicy(*targetStrategy, normInputs, m_DICOMReadPolicy);

  const auto& inputModel = m_EffectiveInputModel.value();

  if (SUVPixelSemantics::ActivityConcentration == inputModel.semantics)
  {
    // Standard SUV pipeline. The input model's activityScale (1.0 for
    // BQML, Philips factor for CNTS+activity) folds into the formula
    // analytically as a pre-multiplier on the scale numerator: the
    // standard expression
    //   SUV = pixel * scaleNum / (activity * 2^(-decay/halfLife))
    // becomes, with pixel' = pixel * activityScale,
    //   SUV = pixel * (scaleNum * activityScale) / (activity * 2^(-decay/halfLife))
    // so we hand the per-voxel functor the scaled numerator instead of
    // multiplying every voxel by activityScale separately.
    const double activity        = GetEffectiveInjectedActivityInBq();
    const double halfLife        = GetEffectiveHalfLifeInSec();
    const double activityScale   = inputModel.activityScale;
    const double scaledNumerator = scaleNumeratorTarget * activityScale;
    const auto& decay = GetEffectiveDecayCorrection();

    auto dispatch = [&](TimeStepType t, const Image* stepIn) {
      const auto it = decay.decayTimes.find(t);
      const auto& sliceMap = (it != decay.decayTimes.end()) ? it->second
                                                            : DecayTimeSliceMapType{};
      ProcessTimeStep(stepIn, outputImage, t,
                      activity, scaledNumerator, halfLife, sliceMap);
    };
    if (1U == inputImage->GetTimeSteps())
    {
      dispatch(0, inputImage);
    }
    else
    {
      for (TimeStepType t = 0; t < inputImage->GetTimeSteps(); ++t)
      {
        auto sel = ImageTimeSelector::New();
        sel->SetInput(inputImage);
        sel->SetTimeNr(t);
        sel->UpdateLargestPossibleRegion();
        dispatch(t, sel->GetOutput());
      }
    }
  }
  else
  {
    // Pre-normalized SUV path: re-scale source variant to target variant.
    //   SUV_target = pixel * prenormScale * scaleNum_target / scaleNum_source
    // No decay correction, dose, or half-life is consumed.
    auto sourceStrategy = MakeSUVNormalizationStrategy(inputModel.sourceVariant);
    const double scaleNumeratorSource =
      ResolveScaleNumeratorUnderSexPolicy(*sourceStrategy, normInputs, m_DICOMReadPolicy);
    if (scaleNumeratorSource <= 0.0)
    {
      mitkThrow() << "SUVImageFilter: source-variant scale numerator must be "
                     "positive (got " << scaleNumeratorSource << ").";
    }
    const double factor = inputModel.prenormScale * scaleNumeratorTarget / scaleNumeratorSource;

    if (1U == inputImage->GetTimeSteps())
    {
      ProcessTimeStepRenormalized(inputImage, outputImage, 0, factor);
    }
    else
    {
      for (TimeStepType t = 0; t < inputImage->GetTimeSteps(); ++t)
      {
        auto sel = ImageTimeSelector::New();
        sel->SetInput(inputImage);
        sel->SetTimeNr(t);
        sel->UpdateLargestPossibleRegion();
        ProcessTimeStepRenormalized(sel->GetOutput(), outputImage, t, factor);
      }
    }
  }

  ApplyOutputTagPolicy(outputImage, m_TargetVariant);
}
