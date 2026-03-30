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

#include <mitkCurveParameterFunctor.h>
#include <mitkDescriptionParameterImageGeneratorBase.h>

#include <MitkPharmacokineticsExports.h>

namespace mitk
{

  /**
   * \brief Generates pixel-wise curve descriptor images from a 4D dynamic image.
   *
   * Uses a CurveParameterFunctor to compute curve description values for each pixel
   * individually. Depending on the registered descriptors in the functor, multiple
   * output parameter images may be generated.
   *
   * An optional mask can restrict computation to a subset of voxels.
   *
   * \remark This generator processes every pixel independently. For ROI-averaged descriptors,
   * use ROIBasedDescriptionParameterImageGenerator instead.
   *
   * \sa DescriptionParameterImageGeneratorBase, CurveParameterFunctor
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
