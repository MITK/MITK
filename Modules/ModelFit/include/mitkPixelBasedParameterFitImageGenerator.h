/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPixelBasedParameterFitImageGenerator_h
#define mitkPixelBasedParameterFitImageGenerator_h

#include <map>

#include <mitkImage.h>

#include "mitkModelParameterizerBase.h"
#include "mitkModelFitFunctorBase.h"
#include "mitkParameterFitImageGeneratorBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /** Class for generators for pixel based parameter fits of a given model based on a given 4D mitk image.
   * The class uses a model fit functor (based on ModelFitFunctorBase) given by the use.
   * @remark This generator fits every pixel on its own. If you want to fit the mean value of the given mask use
   * ROIBasedParameterFitImageGenerator.
   * The generator creates 4 types of images:
   * - parameter images: The images that encode the results of each fitted parameter
   * - derived parameter images: Images that encode the results of derived parameters defined by the model
   * - criterion images: Images that encode the criterion value of the fitting strategy for the fitted parameters
   * - evaluation parameter images: Images that encode measures of additional evaluation cost functions defined by the user. (These were not part of the fitting strategy)
   * .
   */
class MITKMODELFIT_EXPORT PixelBasedParameterFitImageGenerator: public ParameterFitImageGeneratorBase
{
public:
  mitkClassMacro(PixelBasedParameterFitImageGenerator, ParameterFitImageGeneratorBase);

  itkNewMacro(Self);

    typedef ScalarType ParameterImagePixelType;
    typedef std::vector<ParameterImagePixelType> FunctorValueArrayType;

    typedef ModelFitFunctorBase FitFunctorType;

    typedef ModelParameterizerBase ParameterizerType;

    typedef ParameterFitImageGeneratorBase::ModelBaseType ModelBaseType;
    typedef ParameterFitImageGeneratorBase::ParameterNameType ParameterNameType;
    typedef ParameterFitImageGeneratorBase::ParameterImageMapType ParameterImageMapType;

    itkSetObjectMacro(DynamicImage, Image);
    itkGetConstObjectMacro(DynamicImage, Image);

    itkSetObjectMacro(Mask, Image);
    itkGetConstObjectMacro(Mask, Image);

    itkSetObjectMacro(FitFunctor, FitFunctorType);
    itkGetObjectMacro(FitFunctor, FitFunctorType);

    itkSetObjectMacro(ModelParameterizer, ParameterizerType);
    itkGetObjectMacro(ModelParameterizer, ParameterizerType);

    itkSetMacro(TimeGridByParameterizer, bool);
    itkGetMacro(TimeGridByParameterizer, bool);
    itkBooleanMacro(TimeGridByParameterizer);

    double GetProgress() const override;

    ParameterNamesType GetParameterNames() const override;

    ParameterNamesType GetDerivedParameterNames() const override;

    ParameterNamesType GetCriterionNames() const override;

    ParameterNamesType GetEvaluationParameterNames() const override;

protected:
  PixelBasedParameterFitImageGenerator() : m_Progress(0), m_TimeGridByParameterizer(false)
  {
    m_InternalMask = nullptr;
    m_Mask = nullptr;
    m_DynamicImage = nullptr;
  };

  ~PixelBasedParameterFitImageGenerator() override = default;

    template <typename TPixel, unsigned int VDim>
    void DoParameterFit(itk::Image<TPixel, VDim>* image);

    template <typename TPixel, unsigned int VDim>
    void DoPrepareMask(itk::Image<TPixel, VDim>* image);

    void onFitProgressEvent(::itk::Object* caller, const ::itk::EventObject& eventObject);

    bool HasOutdatedResult() const override;
    void CheckValidInputs() const override;
    void DoFitAndGetResults(ParameterImageMapType& parameterImages, ParameterImageMapType& derivedParameterImages, ParameterImageMapType& criterionImages, ParameterImageMapType& evaluationParameterImages) override;

private:
    Image::Pointer m_DynamicImage;
    Image::Pointer m_Mask;

    typedef itk::Image<unsigned char, 3> InternalMaskType;
    InternalMaskType::Pointer m_InternalMask;

    FitFunctorType::Pointer m_FitFunctor;

    ParameterizerType::Pointer m_ModelParameterizer;

    ParameterImageMapType m_TempResultMap;
    ParameterImageMapType m_TempDerivedResultMap;
    ParameterImageMapType m_TempEvaluationResultMap;
    ParameterImageMapType m_TempCriterionResultMap;

    double m_Progress;
    /**Indicates if the time grid defined in the parameterizer should be used (True)
    or if the filter should extract the time grid from the input image (False).*/
    bool m_TimeGridByParameterizer;
};

}

#endif
