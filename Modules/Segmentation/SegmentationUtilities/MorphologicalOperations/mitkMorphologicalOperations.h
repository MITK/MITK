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

#include <mitkImage.h>
#include <MitkSegmentationExports.h>

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
      Ball_Sagital = 2,
      Ball_Coronal = 4,

      Cross = 56,
      Cross_Axial = 8,
      Cross_Sagital = 16,
      Cross_Coronal = 32

    };



    ///@{
    /** \brief Perform morphological operation on 2D, 3D or 3D+t segmentation.
     */
    static void Closing(mitk::Image::Pointer& image, int factor, StructuralElementType structuralElement);
    static void Erode(mitk::Image::Pointer& image, int factor, StructuralElementType structuralElement);
    static void Dilate(mitk::Image::Pointer& image, int factor, StructuralElementType structuralElement);
    static void Opening(mitk::Image::Pointer& image, int factor, StructuralElementType structuralElement);
    static void FillHoles(mitk::Image::Pointer& image);
    ///@}

  private:
    MorphologicalOperations();

    template<class TStructuringElement>
    static TStructuringElement CreateStructuringElement(StructuralElementType structuralElementFlag, int factor)
    {
      TStructuringElement strElem;
      typename TStructuringElement::SizeType size;
      size.Fill(0);
      switch (structuralElementFlag) {
      case Ball_Axial:
      case Cross_Axial:
        size.SetElement(0,factor);
        size.SetElement(1,factor);
        break;
      case Ball_Coronal:
      case Cross_Coronal:
        size.SetElement(0,factor);
        size.SetElement(2,factor);
        break;
      case Ball_Sagital:
      case Cross_Sagital:
        size.SetElement(1,factor);
        size.SetElement(2,factor);
        break;
      case Ball:
      case Cross:
        size.Fill(factor);
        break;
      }

      strElem.SetRadius(size);
      strElem.CreateStructuringElement();
      return strElem;
    }

    ///@{
    /** \brief Perform morphological operation by using corresponding ITK filter.
     */
    template<typename TPixel, unsigned int VDimension>
    void static itkClosing(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkErode(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkDilate(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkOpening(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, StructuralElementType structuralElement);

    template<typename TPixel, unsigned int VDimension>
    void static itkFillHoles(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage);
    ///@}
  };

}

#endif


