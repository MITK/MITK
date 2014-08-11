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

#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkConvert2Dto3DImageFilter.h"

mitk::Convert2Dto3DImageFilter::Convert2Dto3DImageFilter()
{

}

mitk::Convert2Dto3DImageFilter::~Convert2Dto3DImageFilter()
{
}

void mitk::Convert2Dto3DImageFilter::GenerateData()
{
   mitk::Image::ConstPointer inputImage = this->GetInput();
   mitk::Image::Pointer resultImage = this->GetOutput();

  AccessFixedDimensionByItk_1(inputImage, ItkConvert2DTo3D, 2, resultImage);

  resultImage->SetGeometry( inputImage->GetGeometry());
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::Convert2Dto3DImageFilter::ItkConvert2DTo3D(const itk::Image<TPixel,VImageDimension>* itkImage, mitk::Image::Pointer &mitkImage)
{
   typedef itk::Image<TPixel,3> itkImageType3D;
   typedef typename itkImageType3D::SizeType::SizeValueType sizeValType;

   // Create a new ITK image
   typename itkImageType3D::Pointer outputImage = itkImageType3D::New();
   typename itkImageType3D::RegionType myRegion;
   typename itkImageType3D::SizeType mySize;
   typename itkImageType3D::IndexType myIndex;
   typename itkImageType3D::SpacingType mySpacing;
   mySpacing[0] = itkImage->GetSpacing()[0];
   mySpacing[1] = itkImage->GetSpacing()[1];
   mySpacing[2] = 1;
   myIndex[0] = 0;
   myIndex[1] = 0;
   myIndex[2] = 0;
   mySize[0] = itkImage->GetLargestPossibleRegion().GetSize()[0];
   mySize[1] = itkImage->GetLargestPossibleRegion().GetSize()[1];
   mySize[2] = 1;
   myRegion.SetSize( mySize);
   myRegion.SetIndex( myIndex );
   outputImage->SetSpacing(mySpacing);
   outputImage->SetRegions( myRegion);
   outputImage->Allocate();

   // Copy Values:
   for ( sizeValType x = 0; x<mySize[0]; x++)
   {
      for ( sizeValType y = 0; y<mySize[1]; y++)
      {
         itk::Index<2> index2D;
         index2D[0] = x;
         index2D[1] = y;

         itk::Index<3> index3D;
         index3D[0] = x;
         index3D[1] = y;
         index3D[2] = 0;

         outputImage->SetPixel(index3D,
            itkImage->GetPixel(index2D));
      }
   }

   mitk::CastToMitkImage(outputImage, mitkImage);
}

