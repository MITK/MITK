/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIBasedParameterFitImageGenerator_h
#define mitkROIBasedParameterFitImageGenerator_h

#include <map>

#include <mitkImage.h>

#include "mitkModelParameterizerBase.h"
#include "mitkModelFitFunctorBase.h"
#include "mitkParameterFitImageGeneratorBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /** Class for generators for parameter fits of a given model based on one given signal and its time grid.
   * The class uses a model fit functor (based on ModelFitFunctorBase) given by the user. It will fit the model to the passed signal.
   * To produce the output/parameter images, the passed mask will be used as template. The geometry of the mask specifies the geometry
   * of the result images. Every pixel inside the mask (mask value > 0) will be filled with the fitting results. Every pixel outside the mask
   * (mask value == 0) will be set to 0.\n
   * The generators creates 4 types of images:
   * - parameter images: The images that encode the results of each fitted parameter
   * - derived parameter images: Images that encode the results of derived parameters defined by the model
   * - criterion images: Images that encode the criterion value of the fitting strategy for the fitted parameters
   * - evaluation parameter images: Images that encode measures of additional evaluation cost functions defined by the user. (These were not part of the fitting strategy)
   * .
   */
class MITKMODELFIT_EXPORT ROIBasedParameterFitImageGenerator: public ParameterFitImageGeneratorBase
{
public:
  mitkClassMacro(ROIBasedParameterFitImageGenerator, ParameterFitImageGeneratorBase);

  itkNewMacro(Self);

    using ParameterImagePixelType = ScalarType;
    using FunctorValueArrayType = std::vector<ParameterImagePixelType>;

    using FitFunctorType = ModelFitFunctorBase;

    using ParameterizerType = ModelParameterizerBase;

    using ModelBaseType = ParameterFitImageGeneratorBase::ModelBaseType;
    using ParameterNameType = ParameterFitImageGeneratorBase::ParameterNameType;
    using ParameterImageMapType = ParameterFitImageGeneratorBase::ParameterImageMapType;

    using TimeGridType = ModelBaseType::TimeGridType;
    using SignalType = ModelBaseType::ModelResultType;

    itkSetObjectMacro(Mask, Image);
    itkGetConstObjectMacro(Mask, Image);

    itkGetConstReferenceMacro(Signal, SignalType);
    itkSetMacro(Signal, SignalType);

    itkGetConstReferenceMacro(TimeGrid, TimeGridType);
    itkSetMacro(TimeGrid, TimeGridType);

    itkSetObjectMacro(FitFunctor, FitFunctorType);
    itkGetObjectMacro(FitFunctor, FitFunctorType);

    itkSetObjectMacro(ModelParameterizer, ParameterizerType);
    itkGetObjectMacro(ModelParameterizer, ParameterizerType);

    double GetProgress() const override;

    ParameterNamesType GetParameterNames() const override;

    ParameterNamesType GetDerivedParameterNames() const override;

    ParameterNamesType GetCriterionNames() const override;

    ParameterNamesType GetEvaluationParameterNames() const override;

protected:
  ROIBasedParameterFitImageGenerator() : m_Progress(0)
  {
    m_Mask = nullptr;
  };

  ~ROIBasedParameterFitImageGenerator() override = default;

    template <typename TPixel, unsigned int VDim>
    void DoImageGeneration(itk::Image<TPixel, VDim>* image, double value);

    void onFitProgressEvent(::itk::Object* caller, const ::itk::EventObject& eventObject);

    bool HasOutdatedResult() const override;
    void CheckValidInputs() const override;
    void DoFitAndGetResults(ParameterImageMapType& parameterImages, ParameterImageMapType& derivedParameterImages, ParameterImageMapType& criterionImages, ParameterImageMapType& evaluationParameterImages) override;

private:
    Image::Pointer m_Mask;
    SignalType m_Signal;
    TimeGridType m_TimeGrid;

    FitFunctorType::Pointer m_FitFunctor;

    Image::Pointer m_TempResultImage;

    ParameterizerType::Pointer m_ModelParameterizer;

    double m_Progress;
};

}

#endif
