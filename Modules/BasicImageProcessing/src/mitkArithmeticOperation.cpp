/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkArithmeticOperation.h"

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <itkArithmeticOpsFunctors.h>
#include <itkImage.h>
#include "itkUnaryFunctorImageFilter.h"
#include <itkBinaryFunctorImageFilter.h>

#include <itkTanImageFilter.h>
#include <itkAtanImageFilter.h>
#include <itkCosImageFilter.h>
#include <itkAcosImageFilter.h>
#include <itkSinImageFilter.h>
#include <itkAsinImageFilter.h>
#include <itkRoundImageFilter.h>
#include <itkSquareImageFilter.h>
#include <itkSqrtImageFilter.h>
#include <itkAbsImageFilter.h>
#include <itkExpImageFilter.h>
#include <itkExpNegativeImageFilter.h>
#include <itkLog10ImageFilter.h>


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
    template< class TInput, class TOutput>
    class SubValue
    {
    public:
      SubValue() {};
      ~SubValue() {};
      bool operator!=(const SubValue &) const
      {
        return false;
      }
      bool operator==(const SubValue & other) const
      {
        return !(*this != other);
      }
      inline TOutput operator()(const TInput & A) const
      {
        if (valueLeft)
          return value - A;
        else
          return A - value;
      }

      bool valueLeft = false;
      double value = 0.0;
    };
    template< class TInput, class TOutput>
    class MultValue
    {
    public:
      MultValue() {};
      ~MultValue() {};
      bool operator!=(const MultValue &) const
      {
        return false;
      }
      bool operator==(const MultValue & other) const
      {
        return !(*this != other);
      }
      inline TOutput operator()(const TInput & A) const
      {
        return A * value;
      }

      bool valueLeft = false;
      double value = 0.0;
    };
    template< class TInput, class TOutput>
    class DivValue
    {
    public:
      DivValue() {};
      ~DivValue() {};
      bool operator!=(const DivValue &) const
      {
        return false;
      }
      bool operator==(const DivValue & other) const
      {
        return !(*this != other);
      }
      inline TOutput operator()(const TInput & A) const
      {
        if (valueLeft)
          return value / A;
        else
          return A / value;
      }

      bool valueLeft = false;
      double value = 1.0;
    };
    template< class TInput, class TOutput>
    class PowValue
    {
    public:
      PowValue() {};
      ~PowValue() {};
      bool operator!=(const PowValue &) const
      {
        return false;
      }
      bool operator==(const PowValue & other) const
      {
        return !(*this != other);
      }
      inline TOutput operator()(const TInput & A) const
      {
        if (valueLeft)
          return static_cast<TOutput>(std::pow(value, A));
        else
          return static_cast<TOutput>(std::pow(A, value));
      }

      bool valueLeft = false;
      double value = 1.0;
    };
  }
}


template<typename DefaultFunctorType, typename DoubleFunctorType, typename ImageType, typename DoubleImageType >
static void ExecuteOneImageFilterWithFunctor(ImageType* imageA, double value, bool returnDoubleImage, bool valueLeft, bool , mitk::Image::Pointer & outputImage)
{
  typedef itk::UnaryFunctorImageFilter< ImageType, ImageType, DefaultFunctorType > DefaultFilterType;
  typedef itk::UnaryFunctorImageFilter< ImageType, DoubleImageType, DoubleFunctorType > DoubleFilterType;

  if (returnDoubleImage)
  {
    typename DoubleFilterType::Pointer filter = DoubleFilterType::New();
    filter->SetInput(imageA);
    filter->GetFunctor().valueLeft = valueLeft;
    filter->GetFunctor().value = value;
    filter->Update();
    CastToMitkImage(filter->GetOutput(), outputImage);
  }
  else
  {
    typename DefaultFilterType::Pointer filter = DefaultFilterType::New();
    filter->SetInput(imageA);
    filter->GetFunctor().valueLeft = valueLeft;
    filter->GetFunctor().value = value;
    filter->Update();
    CastToMitkImage(filter->GetOutput(), outputImage);
  }
}

template<typename DefaultFunctorType, typename DoubleFunctorType, typename ImageType, typename DoubleImageType >
static void ExecuteOneImageFilterWithFunctorNonParameter(ImageType* imageA, double , bool returnDoubleImage, bool , bool , mitk::Image::Pointer & outputImage)
{
  typedef itk::UnaryFunctorImageFilter< ImageType, ImageType, DefaultFunctorType > DefaultFilterType;
  typedef itk::UnaryFunctorImageFilter< ImageType, DoubleImageType, DoubleFunctorType > DoubleFilterType;

  if (returnDoubleImage)
  {
    typename DoubleFilterType::Pointer filter = DoubleFilterType::New();
    filter->SetInput(imageA);
    filter->Update();
    CastToMitkImage(filter->GetOutput(), outputImage);
  }
  else
  {
    typename DefaultFilterType::Pointer filter = DefaultFilterType::New();
    filter->SetInput(imageA);
    filter->Update();
    CastToMitkImage(filter->GetOutput(), outputImage);
  }
}

template<typename TPixel, unsigned int VImageDimension>
static void ExecuteOneImageFilter(itk::Image<TPixel, VImageDimension>* imageA, double value, bool returnDoubleImage, bool valueLeft, bool parameterFree, mitk::NonStaticArithmeticOperation::OperationsEnum algorithm, mitk::Image::Pointer & outputImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<double, VImageDimension> DoubleOutputType;


  switch (algorithm) {
  case mitk::NonStaticArithmeticOperation::OperationsEnum::AddValue:
    ExecuteOneImageFilterWithFunctor<mitk::Functor::AddValue<TPixel, TPixel>,
                                     mitk::Functor::AddValue<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::SubValue:
    ExecuteOneImageFilterWithFunctor<mitk::Functor::SubValue<TPixel, TPixel>,
                                     mitk::Functor::SubValue<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::MultValue:
      ExecuteOneImageFilterWithFunctor<mitk::Functor::MultValue<TPixel, TPixel>,
                                       mitk::Functor::MultValue<TPixel, double>,
                                       ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::DivValue:
    ExecuteOneImageFilterWithFunctor<mitk::Functor::DivValue<TPixel, TPixel>,
                                     mitk::Functor::DivValue<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::PowValue:
    ExecuteOneImageFilterWithFunctor<mitk::Functor::PowValue<TPixel, TPixel>,
                                     mitk::Functor::PowValue<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;

  case mitk::NonStaticArithmeticOperation::OperationsEnum::Tan:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Tan<TPixel, TPixel>,
                                     itk::Functor::Tan<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::ATan:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Atan<TPixel, TPixel>,
                                     itk::Functor::Atan<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::Cos:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Cos<TPixel, TPixel>,
                                     itk::Functor::Cos<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::ACos:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Acos<TPixel, TPixel>,
                                     itk::Functor::Acos<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::Sin:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Sin<TPixel, TPixel>,
                                     itk::Functor::Sin<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::ASin:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Asin<TPixel, TPixel>,
                                     itk::Functor::Asin<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::Square:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Square<TPixel, TPixel>,
                                     itk::Functor::Square<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::Sqrt:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Sqrt<TPixel, TPixel>,
                                     itk::Functor::Sqrt<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::Abs:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Abs<TPixel, TPixel>,
                                     itk::Functor::Abs<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::Exp:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Exp<TPixel, TPixel>,
                                     itk::Functor::Exp<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::ExpNeg:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::ExpNegative<TPixel, TPixel>,
                                     itk::Functor::ExpNegative<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  case mitk::NonStaticArithmeticOperation::OperationsEnum::Log10:
    ExecuteOneImageFilterWithFunctorNonParameter<itk::Functor::Log10<TPixel, TPixel>,
                                     itk::Functor::Log10<TPixel, double>,
                                     ImageType, DoubleOutputType>(imageA, value, valueLeft, returnDoubleImage, parameterFree, outputImage);
    break;
  default:
    break;
  }
}

mitk::Image::Pointer mitk::ArithmeticOperation::Add(Image::Pointer & imageA, Image::Pointer & imageB, bool)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Add2;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Subtract(Image::Pointer & imageA, Image::Pointer & imageB, bool)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Sub2;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Multiply(Image::Pointer & imageA, Image::Pointer & imageB, bool)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Mult;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Divide(Image::Pointer & imageA, Image::Pointer & imageB, bool)
{
  NonStaticArithmeticOperation helper;
  helper.m_Algorithm = NonStaticArithmeticOperation::OperationsEnum::Div;
  helper.CallExecuteTwoImageFilter(imageA, imageB);
  return helper.m_ResultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Add(Image::Pointer & imageA, double value, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, false, false, NonStaticArithmeticOperation::OperationsEnum::AddValue, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Subtract(Image::Pointer & imageA, double value, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, false, false, NonStaticArithmeticOperation::OperationsEnum::SubValue, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Multiply(Image::Pointer & imageA, double value, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, false, false, NonStaticArithmeticOperation::OperationsEnum::MultValue, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Divide(Image::Pointer & imageA, double value, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, false, false, NonStaticArithmeticOperation::OperationsEnum::DivValue, resultImage));
  return resultImage;
}

mitk::Image::Pointer mitk::ArithmeticOperation::Add(double value, Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, true, false, NonStaticArithmeticOperation::OperationsEnum::AddValue, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Subtract(double value, Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, true, false, NonStaticArithmeticOperation::OperationsEnum::SubValue, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Multiply(double value, Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, true, false, NonStaticArithmeticOperation::OperationsEnum::MultValue, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Divide(double value, Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (value, outputAsDouble, true, false, NonStaticArithmeticOperation::OperationsEnum::DivValue, resultImage));
  return resultImage;
}


mitk::Image::Pointer mitk::ArithmeticOperation::Tan(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Tan, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Atan(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::ATan, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Sin(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Sin, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Asin(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::ASin, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Cos(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Cos, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Acos(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::ACos, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Square(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Square, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Sqrt(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Sqrt, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Abs(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Abs, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Exp(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Exp, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::ExpNeg(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::ExpNeg, resultImage));
  return resultImage;
}
mitk::Image::Pointer mitk::ArithmeticOperation::Log10(Image::Pointer & imageA, bool outputAsDouble)
{
  mitk::Image::Pointer resultImage;
  AccessByItk_n(imageA, ExecuteOneImageFilter, (0.0, outputAsDouble, true, true, NonStaticArithmeticOperation::OperationsEnum::Log10, resultImage));
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
  default:
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
