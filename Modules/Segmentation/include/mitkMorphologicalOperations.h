/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMorphologicalOperations_h
#define mitkMorphologicalOperations_h

#include <MitkSegmentationExports.h>
#include <mitkImage.h>

namespace mitk
{
  /**
   * \brief Provides morphological operations for binary segmentation images.
   *
   * Supports closing, opening, erosion, dilation, and hole filling on 2D, 3D, or 3D+t
   * segmentation images. Structuring elements can be ball- or cross-shaped and can be
   * restricted to specific orientations (axial, sagittal, coronal).
   *
   * \note 2D images do not work with coronal or sagittal structuring elements, as those
   *       require a third dimension.
   *
   * \sa OtsuSegmentationFilter
   */
  class MITKSEGMENTATION_EXPORT MorphologicalOperations
  {
  public:
    /**
     * \brief Defines the type and orientation of the structuring element.
     *
     * Values are bitmask-combinable. For example, Ball = Ball_Axial | Ball_Sagittal | Ball_Coronal.
     */
    enum StructuralElementType
    {
      Ball = 7,            /**< Full 3D ball structuring element. */
      Ball_Axial = 1,      /**< Ball restricted to axial plane. */
      Ball_Sagittal = 2,   /**< Ball restricted to sagittal plane. */
      Ball_Coronal = 4,    /**< Ball restricted to coronal plane. */

      Cross = 56,          /**< Full 3D cross structuring element. */
      Cross_Axial = 8,     /**< Cross restricted to axial plane. */
      Cross_Sagittal = 16, /**< Cross restricted to sagittal plane. */
      Cross_Coronal = 32   /**< Cross restricted to coronal plane. */
    };

    ///@{
    /**
     * \brief Performs a morphological closing on a 2D, 3D, or 3D+t segmentation image.
     * \param[in,out] image The segmentation image to process (modified in-place).
     * \param[in] factor The radius/size of the structuring element.
     * \param[in] structuralElement The type and orientation of the structuring element.
     */
    static void Closing(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);

    /**
     * \brief Performs a morphological erosion on a 2D, 3D, or 3D+t segmentation image.
     * \param[in,out] image The segmentation image to process (modified in-place).
     * \param[in] factor The radius/size of the structuring element.
     * \param[in] structuralElement The type and orientation of the structuring element.
     */
    static void Erode(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);

    /**
     * \brief Performs a morphological dilation on a 2D, 3D, or 3D+t segmentation image.
     * \param[in,out] image The segmentation image to process (modified in-place).
     * \param[in] factor The radius/size of the structuring element.
     * \param[in] structuralElement The type and orientation of the structuring element.
     */
    static void Dilate(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);

    /**
     * \brief Performs a morphological opening on a 2D, 3D, or 3D+t segmentation image.
     * \param[in,out] image The segmentation image to process (modified in-place).
     * \param[in] factor The radius/size of the structuring element.
     * \param[in] structuralElement The type and orientation of the structuring element.
     */
    static void Opening(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);

    /**
     * \brief Fills holes in a binary 2D, 3D, or 3D+t segmentation image.
     * \param[in,out] image The segmentation image to process (modified in-place).
     */
    static void FillHoles(mitk::Image::Pointer &image);
    ///@}

  private:
    MorphologicalOperations();

    template <class TStructuringElement>
    static TStructuringElement CreateStructuringElement(StructuralElementType structuralElementFlag, int factor);

    ///@{
    /** \brief Perform morphological operation by using corresponding ITK filter.
     */
    template <typename TPixel, unsigned int VDimension>
    static void itkClosing(itk::Image<TPixel, VDimension> *sourceImage,
                           mitk::Image::Pointer &resultImage,
                           int factor,
                           StructuralElementType structuralElement);

    template <typename TPixel, unsigned int VDimension>
    static void itkErode(itk::Image<TPixel, VDimension> *sourceImage,
                         mitk::Image::Pointer &resultImage,
                         int factor,
                         StructuralElementType structuralElement);

    template <typename TPixel, unsigned int VDimension>
    static void itkDilate(itk::Image<TPixel, VDimension> *sourceImage,
                          mitk::Image::Pointer &resultImage,
                          int factor,
                          StructuralElementType structuralElement);

    template <typename TPixel, unsigned int VDimension>
    static void itkOpening(itk::Image<TPixel, VDimension> *sourceImage,
                           mitk::Image::Pointer &resultImage,
                           int factor,
                           StructuralElementType structuralElement);

    template <typename TPixel, unsigned int VDimension>
    static void itkFillHoles(itk::Image<TPixel, VDimension> *sourceImage, mitk::Image::Pointer &resultImage);
    ///@}
  };
}

#endif
