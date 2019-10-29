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

#include "mitkImageGenerationHelper.h"
#include "mitkArbitraryTimeGeometry.h"
#include "mitkImageCast.h"


  typedef itk::Image<double,3> FrameITKImageType;
  typedef itk::Image<double,4> DynamicITKImageType;


  mitk::Image::Pointer mitk::ImageGenerationHelper::GenerateTestFrame(unsigned int timePointIndex)
  {
    FrameITKImageType::Pointer image = FrameITKImageType::New();

    FrameITKImageType::IndexType start;
    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y
    start[2] =   0;  // first index on Z

    FrameITKImageType::SizeType  size;
    size[0]  = this->m_DimX;  // size along X
    size[1]  = this->m_DimY;  // size along Y
    size[2]  = this->m_DimZ;  // size along Z

    FrameITKImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( start );

    image->SetRegions( region );
    image->Allocate();

    itk::ImageRegionIterator<FrameITKImageType> it = itk::ImageRegionIterator<FrameITKImageType>(image,image->GetLargestPossibleRegion());

    while (!it.IsAtEnd())
    {
      it.Set(this->m_Curve[timePointIndex]);
      ++it;
    }

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( image.GetPointer() );
    mitkImage->SetVolume( image->GetBufferPointer() );

    return mitkImage;
  }




  mitk::Image::Pointer mitk::ImageGenerationHelper::GenerateDynamicImageMITK()
  {

    unsigned int timeSteps = this->m_Grid.GetSize();

    if(this->m_Curve.GetSize() != this->m_Grid.GetSize())
    {
        itkExceptionMacro("Error. TimeGrid and ConcentrationCurve do not have same size. No Image Generation possible!");

    }
    if(this->m_DimX == 0 && this->m_DimY == 0 && this->m_DimZ == 0)
    {
        itkExceptionMacro("Error. No Dimensions for Image Set!");

    }
    if(this->m_Curve.GetSize() == 0 || this->m_Grid.GetSize() == 0)
    {
        itkExceptionMacro("Error. No Curve/Grid set!");

    }

    mitk::Image::Pointer tempImage = GenerateTestFrame(0);
    mitk::Image::Pointer dynamicImage = mitk::Image::New();

    DynamicITKImageType::Pointer dynamicITKImage = DynamicITKImageType::New();
    DynamicITKImageType::RegionType dynamicITKRegion;
    DynamicITKImageType::PointType dynamicITKOrigin;
    DynamicITKImageType::IndexType dynamicITKIndex;
    DynamicITKImageType::SpacingType dynamicITKSpacing;

    dynamicITKSpacing[0] = tempImage->GetGeometry()->GetSpacing()[0];
    dynamicITKSpacing[1] = tempImage->GetGeometry()->GetSpacing()[1];
    dynamicITKSpacing[2] = tempImage->GetGeometry()->GetSpacing()[2];
    dynamicITKSpacing[3] = 1.0;

    dynamicITKIndex[0] = 0;  // The first pixel of the REGION
    dynamicITKIndex[1] = 0;
    dynamicITKIndex[2] = 0;
    dynamicITKIndex[3] = 0;

    dynamicITKRegion.SetSize( 0,tempImage->GetDimension(0));
    dynamicITKRegion.SetSize( 1,tempImage->GetDimension(1));
    dynamicITKRegion.SetSize( 2,tempImage->GetDimension(2));
    dynamicITKRegion.SetSize( 3,timeSteps);

    dynamicITKRegion.SetIndex( dynamicITKIndex );

    dynamicITKOrigin[0]=tempImage->GetGeometry()->GetOrigin()[0];
    dynamicITKOrigin[1]=tempImage->GetGeometry()->GetOrigin()[1];
    dynamicITKOrigin[2]=tempImage->GetGeometry()->GetOrigin()[2];

    dynamicITKImage->SetOrigin(dynamicITKOrigin);
    dynamicITKImage->SetSpacing(dynamicITKSpacing);
    dynamicITKImage->SetRegions( dynamicITKRegion);
    dynamicITKImage->Allocate();
    dynamicITKImage->FillBuffer(0); //not sure if this is necessary

    // Convert
    mitk::CastToMitkImage(dynamicITKImage, dynamicImage);

    ArbitraryTimeGeometry::Pointer timeGeometry = ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();


    for (unsigned int i = 0; i<timeSteps; ++i)
    {
      mitk::Image::Pointer frameImage = GenerateTestFrame(i);
      mitk::ImageReadAccessor readAccess(frameImage, frameImage->GetVolumeData(0));
      dynamicImage->SetVolume(readAccess.GetData(),i);

      timeGeometry->AppendNewTimeStepClone(frameImage->GetGeometry(),this->m_Grid(i),this->m_Grid(i+1));
    }

    dynamicImage->SetTimeGeometry(timeGeometry);

    return dynamicImage;
  }


