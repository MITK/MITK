/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKBOUNDINGOBJECTCUTTER_TXX
#define MITKBOUNDINGOBJECTCUTTER_TXX

#include "mitkStatusBar.h"
#include "mitkImageToItkMultiplexer.h"

//##Documentation
//## @brief For internal within mitkBoundingObjectCutter.cpp and mitkBoundingObjectCutAndCast.txx only

namespace mitk
{

template < typename TPixel, unsigned int VImageDimension, typename TOutputPixel > 
void BoundingObjectCutter::CutImageWithOutputTypeSelect
  ( itk::Image<TPixel, VImageDimension>* inputItkImage, int boTimeStep, TPixel* dummy, TOutputPixel* dummy2 )
{
  typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
  typedef itk::Image<TOutputPixel, VImageDimension> ItkOutputImageType;
  typedef typename itk::ImageBase<VImageDimension>::RegionType ItkRegionType;
  typedef itk::ImageRegionIteratorWithIndex< ItkInputImageType > ItkInputImageIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< ItkOutputImageType > ItkOutputImageIteratorType;

  if(m_BoundingObject.IsNull())
    return;

  if (inputItkImage == NULL)
  {
    mitk::StatusBar::DisplayErrorText ("An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
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
  itk2vtk(m_InputRequestedRegion.GetIndex(), tmpIndex);
  typename ItkRegionType::IndexType index;
  index.SetIndex(tmpIndex);
  
  // then convert the size
  typename ItkRegionType::SizeType::SizeValueType tmpSize[3];
  itk2vtk(m_InputRequestedRegion.GetSize(), tmpSize);
  typename ItkRegionType::SizeType size;
  size.SetSize(tmpSize);
  
  //create the ITK-image-region out of index and size
  ItkRegionType inputRegionOfInterest(index, size);

  // PART 2: get access to the MITK output image via an ITK image
  typename mitk::ImageToItk<TOutputPixel, VImageDimension>::Pointer outputimagetoitk = mitk::ImageToItk<TOutputPixel, VImageDimension>::New();
  outputimagetoitk->SetInput(m_OutputTimeSelector->GetOutput());
  outputimagetoitk->Update();
  typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

  // PART 3: iterate over input and output using ITK iterators
  
  // create the iterators
  ItkInputImageIteratorType  inputIt( inputItkImage, inputRegionOfInterest );
  ItkOutputImageIteratorType outputIt( outputItkImage, outputItkImage->GetLargestPossibleRegion() );

  // Cut the boundingbox out of the image by iterating through 
  // all pixels and checking if they are inside using IsInside()
  m_OutsidePixelCount = 0;
  m_InsidePixelCount = 0;
  mitk::Point3D p;
  mitk::Geometry3D* inputGeometry = this->GetInput()->GetGeometry();

  TOutputPixel outsideValue = (TOutputPixel) m_OutsideValue;

  //shall we use a fixed value for each inside pixel?
  if (GetUseInsideValue())
  {
    TOutputPixel insideValue  = (TOutputPixel) m_InsideValue;
    // yes, use a fixed value for each inside pixel (create a binary mask of the bounding object)
    for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
    {
      vtk2itk(inputIt.GetIndex(), p);
      inputGeometry->IndexToWorld(p, p);
      if(m_BoundingObject->IsInside(p))
      {
        outputIt.Set(insideValue);
        ++m_InsidePixelCount;
      }
      else
      {
        outputIt.Set(outsideValue);
        ++m_OutsidePixelCount;
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
      if(m_BoundingObject->IsInside(p))
      {
        outputIt.Set( (TOutputPixel) inputIt.Value() );
        ++m_InsidePixelCount;
      }
      else
      {
        outputIt.Set( outsideValue );
        ++m_OutsidePixelCount;
      }
    }
  }
}

} // of namespace mitk
#endif // of MITKBOUNDINGOBJECTCUTTER_TXX




