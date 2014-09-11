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
#include "mitkBoundingObjectToSegmentationFilter.h"
#include "mitkImageCast.h"

#include <itkImageRegionIteratorWithIndex.h>

mitk::BoundingObjectToSegmentationFilter::BoundingObjectToSegmentationFilter()
{
  this->SetNumberOfRequiredInputs(1);

}

mitk::BoundingObjectToSegmentationFilter::~BoundingObjectToSegmentationFilter()
{

}

void mitk::BoundingObjectToSegmentationFilter::SetBoundingObject(mitk::BoundingObject::Pointer boundingObject)
{
  mitk::BoundingObjectGroup* testgroup = dynamic_cast<mitk::BoundingObjectGroup*> (boundingObject.GetPointer());
  if (testgroup)
    m_boundingObjectGroup = testgroup;
  else
  {
    m_boundingObjectGroup = mitk::BoundingObjectGroup::New();
    m_boundingObjectGroup->AddBoundingObject(boundingObject);
  }
}

void mitk::BoundingObjectToSegmentationFilter::GenerateData()
{
  typedef itk::Image<unsigned char, 3> itkImageType;
  mitk::Image::Pointer outputImage = this->GetOutput();
  mitk::Image::ConstPointer inputImage = this->GetInput();
  outputImage->Initialize(inputImage);

  itkImageType::Pointer itkImage;
  CastToItkImage(outputImage, itkImage);
  itkImage->FillBuffer(0);

  for (unsigned int i=0; i<m_boundingObjectGroup->GetCount(); i++)
  {
     //create region for boundingobject
    mitk::BoundingObject::Pointer boundingObject = m_boundingObjectGroup->GetBoundingObjects().at(i);
    mitk::BaseGeometry::Pointer boGeometry = boundingObject->GetGeometry();
    mitk::BaseGeometry::Pointer inputImageGeometry = inputImage->GetSlicedGeometry();
    mitk::BoundingBox::Pointer boToIm = boGeometry->CalculateBoundingBoxRelativeToTransform(inputImageGeometry->GetIndexToWorldTransform());

    mitk::BoundingBox::ConstPointer imgBB = inputImageGeometry->GetBoundingBox();
    mitk::BoundingBox::PointType minImg = imgBB->GetMinimum();
    mitk::BoundingBox::PointType maxImg = imgBB->GetMaximum();

    itkImageType::IndexType boIndex;
    itkImageType::SizeType boSize;

    mitk::BoundingBox::PointType min = boToIm->GetMinimum();
    mitk::BoundingBox::PointType max = boToIm->GetMaximum();

    //check if boundingbox is inside imageregion
    for (int i =0; i<3; i++)
    {
      if (min [i] < minImg[i])
        min[i] = minImg[i];
      if (max [i] < minImg[i])
        max[i] = minImg[i];
      if (max[i] > maxImg[i])
        max[i] = maxImg[i];
      if (min [i] > maxImg[i])
        min[i] = maxImg[i]-1;
    }


    // add 0.5 (boGeometry is no image geometry)
    boIndex[0] = (mitk::SlicedData::IndexType::IndexValueType)(min[0] + 0.5);
    boIndex[1] = (mitk::SlicedData::IndexType::IndexValueType)(min[1] + 0.5);
    boIndex[2] = (mitk::SlicedData::IndexType::IndexValueType)(min[2] + 0.5);

    // add 1 because we need 0.5 for each index
    boSize[0] = (mitk::SlicedData::IndexType::IndexValueType) (max[0]-min[0]);
    boSize[1] = (mitk::SlicedData::IndexType::IndexValueType) (max[1]-min[1]);
    boSize[2] = (mitk::SlicedData::IndexType::IndexValueType) (max[2]-min[2]);

    itkImageType::RegionType region(boIndex, boSize);

    //create region iterator
    itk::ImageRegionIteratorWithIndex<itkImageType> itBoundingObject =  itk::ImageRegionIteratorWithIndex<itkImageType>(itkImage, region );
    itBoundingObject.GoToBegin();

    while(!itBoundingObject.IsAtEnd())
    {
      itkImageType::IndexType index = itBoundingObject.GetIndex();
      mitk::Point3D p;
      p[0] = index[0];
      p[1] = index[1];
      p[2] = index[2];
      inputImageGeometry->IndexToWorld(p,p);

      if (boundingObject->IsInside(p) && boundingObject->GetPositive())
        itBoundingObject.Set(1);
      else if (boundingObject->IsInside(p) && !boundingObject->GetPositive())
        itBoundingObject.Set(0);

      ++itBoundingObject;
    }
  }
  CastToMitkImage(itkImage, outputImage);
}
