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

#ifndef mitkMorphologicalOperations_h
#define mitkMorphologicalOperations_h

#include "SegmentationExports.h"
#include <mitkImage.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>
#include <mitkImageTimeSelector.h>

namespace mitk
{
  class Segmentation_EXPORT MorphologicalOperations
  {
  public:
    MorphologicalOperations();
    ~MorphologicalOperations();

    enum StructuralElementType
    {
       BALL,
       CUBE
    };

    ///@{
    /**
    * \brief Static functions to perform morphological operations on 2D,3D or 4D segmentation images with pixel type unsigned char.
    */
    static void Closing(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
    static void Erode(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
    static void Dilate(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
    static void Opening(mitk::Image::Pointer &image, int factor, StructuralElementType structuralElement);
    static void Fillhole(mitk::Image::Pointer &image);
   ///@}

  protected:
    ///@{
    /**
    * \brief Performs morphological operations using itk filters
    */
    template<typename TPixel, unsigned int VDimension>
    void static itkClosing(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkErode(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkDilate(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkOpening(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkFillhole(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage);
   ///@}
  };
}

#endif // mitkMorphologicalOperations_h
