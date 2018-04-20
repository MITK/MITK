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

#include "mitkArithmeticOperation.h"

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <itkArithmeticOpsFunctors.h>
#include <itkImage.h>
#include "itkUnaryFunctorImageFilter.h"
#include <itkBinaryFunctorImageFilter.h>
namespace mitk
{
  namespace Functor
  {
    template< class TInput, class TOutput>
    class AddValue
    {
    public:
      AddValue() {};
      ~AddValue() {};
      bool operator!=(const AddValue &) const
      {
        return false;
      }
      bool operator==(const AddValue & other) const
      {
        return !(*this != other);
      }
      inline TOutput operator()(const TInput & A) const
      {
        return A + value;
      }

      bool valueLeft = false;
      double value = 0.0;
    };
  }
}

template<typename TPixel, unsigned int VImageDimension>
static void ExecuteOneImageFilter(itk::Image<TPixel, VImageDimension>* imageA, double value, bool returnDoubleImage, bool valueLeft, mitk::NonStaticArithmeticOperation::OperationsEnum algorithm, mitk::Image::Pointer & outputImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<double, VImageDimension> DoubleOutputType;


  switch (algorithm) {
  case OperationsEnum::AddValue:
    ExecuteOneImageFilterWithFunctor<mitk::Functor::AddValue<TPixel, TPixel>,
                                     mitk::Functor::AddValue<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, mitk::Image::Pointer & outputImage);
    break;
  }
}

template<typename DefaultFunctorType, typename DoubleFunctorType, typename ImageType, typename DoubleImageType >
static void ExecuteOneImageFilterWithFunctor(ImageType* imageA, double value, bool returnDoubleImage, bool valueLeft, mitk::Image::Pointer & outputImage)
{
  typedef itk::UnaryFunctorImageFilter< Image1Type, Image1Type, DefaultFunctorType > DefaultFilterType;
  typedef itk::UnaryFunctorImageFilter< Image1Type, DoubleImageType, DoubleFunctorType > DoubleFilterType;

  if (returnDoubleImage)
  {
    DoubleFunctorType functor;
    functor.valueLeft = valueLeft;
    functor.value = value;
    typename DoubleFilterType::Pointer filter = DoubleFilterType::New();
    filter->SetInput1(imageA);
    filter->SetFunctor(functor);
    filter->Update();
    CastToMitkImage(filter->GetOutput(), outputImage);
  }
  else
  {
    DefaultFunctorType functor;
    functor.valueLeft = valueLeft;
    functor.value = value;
    typename DefaultFilterType::Pointer filter = DefaultFilterType::New();
    filter->SetInput1(imageA);
    filter->SetFunctor(functor);
    filter->Update();
    CastToMitkImage(filter->GetOutput(), outputImage);
  }
}


mitk::Image::Pointer mitk::ArithmeticOperation::Add(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Add2;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Subtract(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Sub2;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Multiply(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Mult;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Divide(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Div;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Add(Image::Pointer & imageA, double value, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, false, NonStaticArithmeticOperation::OperationsEnum::AddValue, resultImage));
  return resultImage;
}


void mitk::NonStaticArithmeticOperation::CallExecuteTwoImageFilter(mitk::Image::Pointer imageA, mitk::Image::Pointer imageB)
{
  if (imageA->GetDimension() != imageB->GetDimension())
  {
    mitkThrow() << "Image have different dimensions. This is not supported by mitk::ArithmeticOperation";
  }

  switch (imageA->GetDimension())
  {
  case 1:
    AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::NonStaticArithmeticOperation::ExecuteTwoImageFilter, 1);
    break;
  case 2:
    AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::NonStaticArithmeticOperation::ExecuteTwoImageFilter, 2);
    break;
  case 3:
    AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::NonStaticArithmeticOperation::ExecuteTwoImageFilter, 3);
    break;
  case 4:
    AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::NonStaticArithmeticOperation::ExecuteTwoImageFilter, 4);
    break;
  default:
    mitkThrow() << "Image Dimension of "<<imageA->GetDimension() << " is not supported";
    break;
  }
}


template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
void mitk::NonStaticArithmeticOperation::ExecuteTwoImageFilter(itk::Image<TPixel1, VImageDimension1>* imageA, itk::Image<TPixel2, VImageDimension2>* imageB)
{
  typedef itk::Image<TPixel1, VImageDimension1> Image1Type;
  typedef itk::Image<TPixel2, VImageDimension2> Image2Type;
  typedef itk::Image<double, VImageDimension1> DoubleOutputType;


  switch (m_Algorithm) {
  case OperationsEnum::Add2:
    ExecuteTwoImageFilterWithFunctor<itk::Functor::Add2<TPixel1, TPixel2, TPixel1>,
                                     itk::Functor::Add2<TPixel1, TPixel2, double>,
                                     Image1Type, Image2Type, DoubleOutputType>(imageA, imageB);
    break;

  case OperationsEnum::Sub2:
    ExecuteTwoImageFilterWithFunctor<itk::Functor::Sub2<TPixel1, TPixel2, TPixel1>,
      itk::Functor::Add2<TPixel1, TPixel2, double>,
      Image1Type, Image2Type, DoubleOutputType>(imageA, imageB);
    break;

  case OperationsEnum::Mult:
    ExecuteTwoImageFilterWithFunctor<itk::Functor::Mult<TPixel1, TPixel2, TPixel1>,
      itk::Functor::Add2<TPixel1, TPixel2, double>,
      Image1Type, Image2Type, DoubleOutputType>(imageA, imageB);
    break;

  case OperationsEnum::Div:
    ExecuteTwoImageFilterWithFunctor<itk::Functor::Div<TPixel1, TPixel2, TPixel1>,
      itk::Functor::Add2<TPixel1, TPixel2, double>,
      Image1Type, Image2Type, DoubleOutputType>(imageA, imageB);
    break;
  }
}

template<typename DefaultFunctorType, typename DoubleFunctorType, typename Image1Type, typename Image2Type, typename DoubleImageType >
void mitk::NonStaticArithmeticOperation::ExecuteTwoImageFilterWithFunctor(Image1Type* imageA, Image2Type* imageB)
{
  typedef itk::BinaryFunctorImageFilter< Image1Type, Image2Type, Image1Type,DefaultFunctorType > DefaultFilterType;
  typedef itk::BinaryFunctorImageFilter< Image1Type, Image2Type, DoubleImageType, DoubleFunctorType > DoubleFilterType;

  if (m_GenerateDoubleOutput)
  {
    typename DoubleFilterType::Pointer filter = DoubleFilterType::New();
    filter->SetInput1(imageA);
    filter->SetInput2(imageB);
    filter->Update();
    CastToMitkImage(filter->GetOutput(), m_ResultImage);
  }
  else
  {
    typename DefaultFilterType::Pointer filter = DefaultFilterType::New();
    filter->SetInput1(imageA);
    filter->SetInput2(imageB);
    filter->Update();
    CastToMitkImage(filter->GetOutput(), m_ResultImage);
  }
}