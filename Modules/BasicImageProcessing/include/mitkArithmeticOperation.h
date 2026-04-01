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
  /**
  \brief Provides static methods for voxel-wise arithmetic operations on images.

  The ArithmeticOperation class offers a comprehensive set of static methods for performing
  element-wise arithmetic on mitk::Image objects. Operations include basic arithmetic
  (add, subtract, multiply, divide) between two images or between an image and a scalar,
  as well as unary mathematical functions (trigonometric, exponential, logarithmic, etc.).

  All operations support an optional flag to produce output with double pixel type.
  Two-image operations require both images to have the same dimensionality
  (1D, 2D, 3D, or 4D are supported).

  \sa TransformationOperation, MaskCleaningOperation
  */
  class MITKBASICIMAGEPROCESSING_EXPORT ArithmeticOperation {
  public:
    /** \name Two-Image Arithmetic Operations
     *
     * Perform voxel-wise arithmetic between two images. Both images must have
     * the same dimensionality.
     */
    ///@{

    /**
    \brief Voxel-wise addition of two images: result = imageA + imageB.
    \param[in] imageA First input image.
    \param[in] imageB Second input image (must have same dimensions as imageA).
    \param[in] outputAsDouble If true, the output image has double pixel type (currently unused in two-image operations).
    \return A new image containing the voxel-wise sum.
    \throw mitk::Exception if the images have different dimensionalities.
    */
    static Image::Pointer Add(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);

    /**
    \brief Voxel-wise subtraction of two images: result = imageA - imageB.
    \param[in] imageA First input image.
    \param[in] imageB Second input image (must have same dimensions as imageA).
    \param[in] outputAsDouble If true, the output image has double pixel type (currently unused in two-image operations).
    \return A new image containing the voxel-wise difference.
    \throw mitk::Exception if the images have different dimensionalities.
    */
    static Image::Pointer Subtract(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);

    /**
    \brief Voxel-wise multiplication of two images: result = imageA * imageB.
    \param[in] imageA First input image.
    \param[in] imageB Second input image (must have same dimensions as imageA).
    \param[in] outputAsDouble If true, the output image has double pixel type (currently unused in two-image operations).
    \return A new image containing the voxel-wise product.
    \throw mitk::Exception if the images have different dimensionalities.
    */
    static Image::Pointer Multiply(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);

    /**
    \brief Voxel-wise division of two images: result = imageA / imageB.
    \param[in] imageA First input image (numerator).
    \param[in] imageB Second input image (denominator, must have same dimensions as imageA).
    \param[in] outputAsDouble If true, the output image has double pixel type (currently unused in two-image operations).
    \return A new image containing the voxel-wise quotient.
    \throw mitk::Exception if the images have different dimensionalities.
    */
    static Image::Pointer Divide(Image::Pointer & imageA, Image::Pointer & imageB, bool outputAsDouble = true);
    ///@}

    /** \name Image-Scalar Arithmetic Operations
     *
     * Perform voxel-wise arithmetic between an image and a scalar value.
     * The scalar is applied to each voxel: result(x) = imageA(x) op value.
     */
    ///@{

    /**
    \brief Add a scalar value to each voxel: result = imageA + value.
    \param[in] imageA The input image.
    \param[in] value The scalar value to add.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the scalar added to each voxel.
    */
    static Image::Pointer Add(Image::Pointer & imageA, double value, bool outputAsDouble = true);

    /**
    \brief Subtract a scalar value from each voxel: result = imageA - value.
    \param[in] imageA The input image.
    \param[in] value The scalar value to subtract.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the scalar subtracted from each voxel.
    */
    static Image::Pointer Subtract(Image::Pointer & imageA, double value, bool outputAsDouble = true);

    /**
    \brief Multiply each voxel by a scalar value: result = imageA * value.
    \param[in] imageA The input image.
    \param[in] value The scalar multiplier.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with each voxel multiplied by the scalar.
    */
    static Image::Pointer Multiply(Image::Pointer & imageA, double value, bool outputAsDouble = true);

    /**
    \brief Divide each voxel by a scalar value: result = imageA / value.
    \param[in] imageA The input image.
    \param[in] value The scalar divisor.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with each voxel divided by the scalar.
    */
    static Image::Pointer Divide(Image::Pointer & imageA, double value, bool outputAsDouble = true);
    ///@}

    /** \name Scalar-Image Arithmetic Operations
     *
     * Perform voxel-wise arithmetic with a scalar on the left side:
     * result(x) = value op imageB(x). The order matters for subtraction and division.
     */
    ///@{

    /**
    \brief Add a scalar value to each voxel (commutative): result = value + imageB.
    \param[in] value The scalar value.
    \param[in] imageB The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the scalar added to each voxel.
    */
    static Image::Pointer Add(double value, Image::Pointer & imageB, bool outputAsDouble = true);

    /**
    \brief Subtract each voxel from a scalar: result = value - imageB.
    \param[in] value The scalar value (minuend).
    \param[in] imageB The input image (subtrahend).
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image where each voxel is value minus the original voxel.
    */
    static Image::Pointer Subtract(double value, Image::Pointer & imageB, bool outputAsDouble = true);

    /**
    \brief Multiply a scalar by each voxel (commutative): result = value * imageB.
    \param[in] value The scalar multiplier.
    \param[in] imageB The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with each voxel multiplied by the scalar.
    */
    static Image::Pointer Multiply(double value, Image::Pointer & imageB, bool outputAsDouble = true);

    /**
    \brief Divide a scalar by each voxel: result = value / imageB.
    \param[in] value The scalar numerator.
    \param[in] imageB The input image (denominator).
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image where each voxel is value divided by the original voxel.
    */
    static Image::Pointer Divide(double value, Image::Pointer & imageB, bool outputAsDouble = true);
    ///@}

    /** \name Unary Mathematical Operations
     *
     * Apply a mathematical function to each voxel of the input image.
     */
    ///@{

    /**
    \brief Compute the tangent of each voxel: result = tan(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the tangent applied to each voxel.
    */
    static Image::Pointer Tan(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the arctangent of each voxel: result = atan(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the arctangent applied to each voxel.
    */
    static Image::Pointer Atan(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the cosine of each voxel: result = cos(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the cosine applied to each voxel.
    */
    static Image::Pointer Cos(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the arccosine of each voxel: result = acos(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the arccosine applied to each voxel.
    */
    static Image::Pointer Acos(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the sine of each voxel: result = sin(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the sine applied to each voxel.
    */
    static Image::Pointer Sin(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the arcsine of each voxel: result = asin(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the arcsine applied to each voxel.
    */
    static Image::Pointer Asin(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the square of each voxel: result = imageA^2.
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with each voxel squared.
    */
    static Image::Pointer Square(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the square root of each voxel: result = sqrt(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the square root applied to each voxel.
    */
    static Image::Pointer Sqrt(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the absolute value of each voxel: result = |imageA|.
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the absolute value of each voxel.
    */
    static Image::Pointer Abs(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the exponential of each voxel: result = exp(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the exponential applied to each voxel.
    */
    static Image::Pointer Exp(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the negative exponential of each voxel: result = exp(-imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the negative exponential applied to each voxel.
    */
    static Image::Pointer ExpNeg(Image::Pointer & imageA, bool outputAsDouble = true);

    /**
    \brief Compute the base-10 logarithm of each voxel: result = log10(imageA).
    \param[in] imageA The input image.
    \param[in] outputAsDouble If true, the output image has double pixel type.
    \return A new image with the base-10 logarithm applied to each voxel.
    */
    static Image::Pointer Log10(Image::Pointer & imageA, bool outputAsDouble = true);
    ///@}
 };

  /**
  \brief Internal helper class for executing two-image arithmetic operations with ITK filters.

  This class provides the non-static infrastructure needed to dispatch two-image arithmetic
  operations through the ITK image access macros. It handles dimension-matching, pixel-type
  casting, and functor-based binary filter execution.

  \note This class is intended for internal use by ArithmeticOperation. Use the static methods
        of ArithmeticOperation instead.

  \sa ArithmeticOperation
  */
  class MITKBASICIMAGEPROCESSING_EXPORT NonStaticArithmeticOperation {
  public:
    /**
    \brief Enumeration of all supported arithmetic operations.
    */
    enum OperationsEnum
    {
      Add2,         ///< Two-image addition
      Sub2,         ///< Two-image subtraction
      Mult,         ///< Two-image multiplication
      Div,          ///< Two-image division
      AddValue,     ///< Add scalar value to image
      SubValue,     ///< Subtract scalar value from image (or image from scalar)
      MultValue,    ///< Multiply image by scalar value
      DivValue,     ///< Divide image by scalar value (or scalar by image)
      PowValue,     ///< Raise image to scalar power (or scalar to image power)
      Tan,          ///< Tangent
      ATan,         ///< Arctangent
      Cos,          ///< Cosine
      ACos,         ///< Arccosine
      Sin,          ///< Sine
      ASin,         ///< Arcsine
      Square,       ///< Square (x^2)
      Sqrt,         ///< Square root
      Abs,          ///< Absolute value
      Exp,          ///< Exponential (e^x)
      ExpNeg,       ///< Negative exponential (e^{-x})
      Log10         ///< Base-10 logarithm
    };

    /**
    \brief Execute a two-image filter operation using ITK access macros.

    Dispatches to the appropriate ITK access macro based on the image dimensionality
    (1D through 4D).

    \param[in] imageA The first input image.
    \param[in] imageB The second input image (must have same dimensionality as imageA).
    \throw mitk::Exception if the images have different dimensionalities or unsupported dimension.
    */
    void CallExecuteTwoImageFilter(mitk::Image::Pointer imageA, mitk::Image::Pointer imageB);

    /**
    \brief Internal templated method that executes the two-image filter with matched pixel types.
    \tparam TPixel1 Pixel type of the first image.
    \tparam VImageDimension1 Dimensionality of the first image.
    \tparam TPixel2 Pixel type of the second image.
    \tparam VImageDimension2 Dimensionality of the second image.
    \param[in] imageA Pointer to the first ITK image.
    \param[in] imageB Pointer to the second ITK image.
    */
    template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void ExecuteTwoImageFilter(itk::Image<TPixel1, VImageDimension1>* imageA, itk::Image<TPixel2, VImageDimension2>* imageB);

    /**
    \brief Internal templated method that applies a functor-based binary filter to two images.
    \tparam DefaultFunctorType The functor type producing same-type output as the first input.
    \tparam DoubleFunctorType The functor type producing double output.
    \tparam Image1Type ITK image type of the first input.
    \tparam Image2Type ITK image type of the second input.
    \tparam DoubleImageType ITK image type for double output.
    \param[in] imageA Pointer to the first ITK image.
    \param[in] imageB Pointer to the second ITK image.
    */
    template<typename DefaultFunctorType, typename DoubleFunctorType, typename Image1Type, typename Image2Type, typename DoubleImageType>
    void ExecuteTwoImageFilterWithFunctor(Image1Type* imageA, Image2Type* imageB);

    /** \brief The result image produced by the most recent filter execution. */
    mitk::Image::Pointer m_ResultImage;
    /** \brief The algorithm/operation to execute. */
    OperationsEnum m_Algorithm;
    /** \brief If true, produce output with double pixel type instead of the input pixel type. */
    bool m_GenerateDoubleOutput = false;
  };


}
#endif
