/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPixelBasedDescriptionParameterImageGenerator_h
#define mitkPixelBasedDescriptionParameterImageGenerator_h

#include <map>

#include <mitkImage.h>

#include "mitkCurveParameterFunctor.h"
#include "mitkDescriptionParameterImageGeneratorBase.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  /** Class for generating curve descriptor images based on a given 4D mitk image.
   * The class uses curve parameter functor to generate the curve description image(s).
   * Depending on the chosen functor several images may be generated as output.
   * @remark This generator fits every pixel on its own. If you want to fit the mean value of the given mask use
   * ROIBasedDescriptionParameterImageGenerator.
   */
  class MITKPHARMACOKINETICS_EXPORT PixelBasedDescriptionParameterImageGenerator : public DescriptionParameterImageGeneratorBase
  {
  public:
    mitkClassMacro(PixelBasedDescriptionParameterImageGenerator, DescriptionParameterImageGeneratorBase);

    itkNewMacro(Self);


    typedef ScalarType ParameterImagePixelType;
    typedef std::vector<ParameterImagePixelType> FunctorValueArrayType;

    typedef CurveParameterFunctor FunctorType;

    typedef DescriptionParameterImageGeneratorBase::ParameterNameType ParameterNameType;
    typedef DescriptionParameterImageGeneratorBase::ParameterImageMapType ParameterImageMapType;

    itkSetObjectMacro(DynamicImage, Image);
    itkGetConstObjectMacro(DynamicImage, Image);

    itkSetObjectMacro(Mask, Image);
    itkGetConstObjectMacro(Mask, Image);

    itkSetObjectMacro(Functor, FunctorType);
    itkGetObjectMacro(Functor, FunctorType);

    double GetProgress() const override;

  protected:
    PixelBasedDescriptionParameterImageGenerator() : m_Progress(0)
    {
      m_InternalMask = nullptr;
      m_Mask = nullptr;
      m_DynamicImage = nullptr;
    };

    ~PixelBasedDescriptionParameterImageGenerator() override {};

    template <typename TPixel, unsigned int VDim>
    void DoParameterCalculation(itk::Image<TPixel, VDim>* image);

    template <typename TPixel, unsigned int VDim>
    void DoPrepareMask(itk::Image<TPixel, VDim>* image);

    void onFitProgressEvent(::itk::Object* caller, const ::itk::EventObject& eventObject);

    bool HasOutdatedResult() const override;
    void CheckValidInputs() const override;
    void DoParameterCalculationAndGetResults(ParameterImageMapType& parameterImages) override;

  private:
    Image::Pointer m_DynamicImage;
    Image::Pointer m_Mask;

    typedef itk::Image<unsigned char, 3> InternalMaskType;
    InternalMaskType::Pointer m_InternalMask;

    FunctorType::Pointer m_Functor;


    ParameterImageMapType m_TempResultMap;

    double m_Progress;
  };

}

#endif
