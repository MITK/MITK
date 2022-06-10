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
  /** \brief Encapsulates several morphological operations that can be performed on segmentations.
    */
  class MITKSEGMENTATION_EXPORT MorphologicalOperations
  {
  public:
    enum StructuralElementType
    {
      Ball = 7,
      Ball_Axial = 1,
      Ball_Sagittal = 2,
      Ball_Coronal = 4,

      Cross = 56,
      Cross_Axial = 8,
      Cross_Sagittal = 16,
      Cross_Coronal = 32

    };

    ///@{
    /** \brief Perform morphological operation on 2D, 3D or 3D+t segmentation.
     */
    static void Closing(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
    static void Erode(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
    static void Dilate(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
    static void Opening(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
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
