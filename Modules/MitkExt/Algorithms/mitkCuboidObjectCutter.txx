/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkCuboidObjectCutter.h>
#include <mitkStatusBar.h>
#include <mitkImageToItk.h>

#include <itkImageRegionIteratorWithIndex.h>

namespace mitk
{
template < typename TPixel, unsigned int VImageDimension, typename TOutputPixel >
void CuboidObjectCutter::CutImageWithOutputTypeSelect( itk::Image< TPixel, VImageDimension >* inputItkImage, mitk::CuboidObjectCutter* cutter, int  /*boTimeStep*/, TOutputPixel*  /*dummy*/ )
{
//  Cuboid *cuboid = static_cast<Cuboid*>(const_cast<BoundingObject*>(cutter->GetBoundingObject()));

  typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
  typedef itk::Image<TOutputPixel, VImageDimension> ItkOutputImageType;
  typedef typename itk::ImageBase<VImageDimension>::RegionType ItkRegionType;
  typedef itk::ImageRegionIteratorWithIndex< ItkInputImageType > ItkInputImageIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< ItkOutputImageType > ItkOutputImageIteratorType;
  typedef typename ItkInputImageType::IndexType IndexType;

  if(cutter->m_BoundingObject.IsNull())
    return;

  if (inputItkImage == NULL)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText ("An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
    std::cout << " image is NULL...returning" << std::endl;
    return;
  }

  // PART 1: convert m_InputRequestedRegion (type mitk::SlicedData::RegionType)
  // into ITK-image-region (ItkImageType::RegionType)
  // unfortunately, we cannot use input->GetRequestedRegion(), because it
  // has been destroyed by the mitk::CastToItkImage call of PART 1
  // (which sets the m_RequestedRegion to the LargestPossibleRegion).
  // Thus, use our own member m_InputRequestedRegion insead.

  // first convert the index
  typename ItkRegionType::IndexType::IndexValueType tmpIndex[3];
  itk2vtk(cutter->m_InputRequestedRegion.GetIndex(), tmpIndex);
  typename ItkRegionType::IndexType index;
  index.SetIndex(tmpIndex);

  // then convert the size
  typename ItkRegionType::SizeType::SizeValueType tmpSize[3];
  itk2vtk(cutter->m_InputRequestedRegion.GetSize(), tmpSize);
  typename ItkRegionType::SizeType size;
  size.SetSize(tmpSize);

  //create the ITK-image-region out of index and size
  ItkRegionType inputRegionOfInterest(index, size);

  // PART 2: get access to the MITK output image via an ITK image
  typename mitk::ImageToItk<ItkOutputImageType>::Pointer outputimagetoitk = mitk::ImageToItk<ItkOutputImageType>::New();
  outputimagetoitk->SetInput(cutter->m_OutputTimeSelector->GetOutput());
  outputimagetoitk->Update();
  typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

  // PART 3: iterate over input and output using ITK iterators

  // create the iterators
  ItkInputImageIteratorType  inputIt( inputItkImage, inputRegionOfInterest );
  ItkOutputImageIteratorType outputIt( outputItkImage, outputItkImage->GetLargestPossibleRegion() );

  // Cut the boundingbox out of the image by iterating through
  // all pixels and checking if they are inside using IsInside()
  cutter->m_OutsidePixelCount = 0;
  cutter->m_InsidePixelCount = 0;
  mitk::Point3D p;
  mitk::Geometry3D
    *inputGeometry = cutter->GetInput()->GetGeometry(),
    *bbGeometry = cutter->m_BoundingObject->GetGeometry();

  //If the bounding object is a cuboid, there is a optimized version for it

  mitk::Vector3D
    iv0 = inputGeometry->GetAxisVector(0),
    iv1 = inputGeometry->GetAxisVector(1),
    iv2 = inputGeometry->GetAxisVector(2),
    bv0 = bbGeometry->GetAxisVector(0),
    bv1 = bbGeometry->GetAxisVector(1),
    bv2 = bbGeometry->GetAxisVector(2);

  iv0.Normalize();
  iv1.Normalize();
  iv2.Normalize();
  bv0.Normalize();
  bv1.Normalize();
  bv2.Normalize();

  //Tests if the bounding box was not rotated: if true, a faster version of the method is executed.
  if ((iv0 == bv0) && (iv1 == bv1) && (iv2 == bv2))
  {
    //shall we use a fixed value for each inside pixel?
    if (cutter->GetUseInsideValue())
    {
      TOutputPixel insideValue  = (TOutputPixel) cutter->m_InsideValue;
      // yes, use a fixed value for each inside pixel (create a binary mask of the bounding object)
      for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        outputIt.Set(insideValue);
      }
    }
    else
    {
      // no, use the pixel value of the original image (normal cutting)
      for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        outputIt.Set( (TOutputPixel) inputIt.Value() );
      }
    }
  }
  else
  {
    if (cutter->GetUseInsideValue())
    {
      TOutputPixel insideValue  = (TOutputPixel) cutter->m_InsideValue;
      // yes, use a fixed value for each inside pixel (create a binary mask of the bounding object)
      for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        vtk2itk(inputIt.GetIndex(), p);
        inputGeometry->IndexToWorld(p, p);
        if(cutter->m_BoundingObject->IsInside(p))
        {
          outputIt.Set(insideValue);
          ++cutter->m_InsidePixelCount;
        }
        else
        {
          outputIt.Set(cutter->m_OutsideValue);
          ++cutter->m_OutsidePixelCount;
        }
      }
    }
    else
    {
      // no, use the pixel value of the original image (normal cutting)
      for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
      {
        vtk2itk(inputIt.GetIndex(), p);
        inputGeometry->IndexToWorld(p, p);
        if(cutter->m_BoundingObject->IsInside(p))
        {
          outputIt.Set( (TOutputPixel) inputIt.Value() );
          ++cutter->m_InsidePixelCount;
        }
        else
        {
          outputIt.Set( cutter->m_OutsideValue );
          ++cutter->m_OutsidePixelCount;
        }
      }
    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
void CuboidObjectCutter::CutImage( itk::Image< TPixel, VImageDimension >* inputItkImage, mitk::CuboidObjectCutter* cutter, int boTimeStep )
{
  TPixel* dummy = NULL;
  CuboidObjectCutter::CutImageWithOutputTypeSelect<TPixel, VImageDimension, TPixel>(inputItkImage, cutter, boTimeStep, dummy);
}
}
