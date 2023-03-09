/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkArithmeticOperation_h
#define mitkArithmeticOperation_h

#include <mitkImage.h>
#include <MitkBasicImageProcessingExports.h>

namespace mitk
{
  /** \brief Executes a arithmetic operations on one or two images
  *
  * All parameters of the arithmetic operations must be specified during construction.
  * The actual operation is executed when calling GetResult().
  */
  class MITKBASICIMAGEPROCESSING_EXPORT ArithmeticOperation {
  public:
    static Image::Pointer Add(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);
    static Image::Pointer Subtract(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);
    static Image::Pointer Multiply(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);
    static Image::Pointer Divide(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);

    static Image::Pointer Add(Image::Pointer & imageA, double value, bool outputAsDouble = true);
    static Image::Pointer Subtract(Image::Pointer & imageA, double value, bool outputAsDouble = true);
    static Image::Pointer Multiply(Image::Pointer & imageA, double value, bool outputAsDouble = true);
    static Image::Pointer Divide(Image::Pointer & imageA, double value, bool outputAsDouble = true);

    static Image::Pointer Add(double value, Image::Pointer & imageB, bool outputAsDouble = true);
    static Image::Pointer Subtract(double value, Image::Pointer & imageB, bool outputAsDouble = true);
    static Image::Pointer Multiply(double value, Image::Pointer & imageB, bool outputAsDouble = true);
    static Image::Pointer Divide(double value, Image::Pointer & imageB, bool outputAsDouble = true);

    static Image::Pointer Tan(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Atan(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Cos(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Acos(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Sin(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Asin(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Square(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Sqrt(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Abs(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Exp(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer ExpNeg(Image::Pointer & imageA, bool outputAsDouble = true);
    static Image::Pointer Log10(Image::Pointer & imageA, bool outputAsDouble = true);
 };

  class MITKBASICIMAGEPROCESSING_EXPORT NonStaticArithmeticOperation {
  public:
    enum OperationsEnum
    {
      Add2,
      Sub2,
      Mult,
      Div,
      AddValue,
      SubValue,
      MultValue,
      DivValue,
      PowValue,
      Tan,
      ATan,
      Cos,
      ACos,
      Sin,
      ASin,
      Square,
      Sqrt,
      Abs,
      Exp,
      ExpNeg,
      Log10
    };


    void CallExecuteTwoImageFilter(mitk::Image::Pointer imageA, mitk::Image::Pointer imageB);

    template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ExecuteTwoImageFilter(itk::Image<TPixel1, VImageDimension1>* imageA, itk::Image<TPixel2, VImageDimension2>* imageB);

    template<typename DefaultFunctorType, typename DoubleFunctorType, typename Image1Type, typename Image2Type, typename DoubleImageType>
    void ExecuteTwoImageFilterWithFunctor(Image1Type* imageA, Image2Type* imageB);

    mitk::Image::Pointer m_ResultImage;
    OperationsEnum m_Algorithm;
    bool m_GenerateDoubleOutput = false;
  };


}
#endif
