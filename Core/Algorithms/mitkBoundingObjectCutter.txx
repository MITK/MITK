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

#include "mitkBoundingObjectCutter.h"
#include "mitkImageToItkMultiplexer.h"
#include "mitkImage.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"
#include "mitkImageToItk.h"
#include "mitkStatusBar.h"

#include <vtkTransform.h>

#include "ipPic.h"

namespace mitk
{

template <typename TPixel>
void BoundingObjectCutter<TPixel>::SetBoundingObject( const mitk::BoundingObject* boundingObject ) 
{
  m_BoundingObject = const_cast<mitk::BoundingObject*>(boundingObject);
	// Process object is not const-correct so the const_cast is required here
	this->ProcessObject::SetNthInput(1, 
		const_cast< mitk::BoundingObject * >( boundingObject ) );
}

template <typename TPixel>
const mitk::BoundingObject* BoundingObjectCutter<TPixel>::GetBoundingObject() const 
{
  return m_BoundingObject.GetPointer();
}

template <typename TPixel>
BoundingObjectCutter<TPixel>::BoundingObjectCutter() : 
m_BoundingObject(NULL), m_InsideValue(1), m_OutsideValue(0), 
m_UseInsideValue(false), m_OutsidePixelCount(0), m_InsidePixelCount(0)
{
  this->SetNumberOfInputs(2);
  this->SetNumberOfRequiredInputs(2);
}

template <typename TPixel>
BoundingObjectCutter<TPixel>::~BoundingObjectCutter()
{
}

template <typename TPixel>
void BoundingObjectCutter<TPixel>::GenerateInputRequestedRegion()
{
  // we have already calculated the m_InputRequestedRegion in 
  // GenerateOutputInformation (which is called before 
  // GenerateInputRequestedRegion)
  const_cast< mitk::Image * > ( this->GetInput() )->SetRequestedRegion(&m_InputRequestedRegion);
}

template <typename TPixel>
void BoundingObjectCutter<TPixel>::GenerateOutputInformation()
{
  mitk::Image::Pointer output = this->GetOutput();
  if ((output->IsInitialized()) && (output->GetPipelineMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  mitk::Image::Pointer input = const_cast< mitk::Image * > ( this->GetInput() );

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(input.IsNull())
    return;  

  if(m_BoundingObject.IsNull())
    return;

  mitk::Geometry3D* boGeometry =  m_BoundingObject->GetGeometry(); 
  mitk::Geometry3D* inputImageGeometry = input->GetSlicedGeometry();
  // calculate bounding box of bounding-object relative to the geometry 
  // of the input image. The result is in pixel coordinates of the input
  // image (because the m_IndexToWorldTransform includes the spacing).
  mitk::BoundingBox::Pointer boBoxRelativeToImage = boGeometry->CalculateBoundingBoxRelativeToTransform( inputImageGeometry->GetIndexToWorldTransform() );
   
   //mitk::Geometry3D::Pointer outputImageGeometry = mitk::Geometry3D::New();   
   //outputImageGeometry->SetIndexToWorldTransform( inputImageGeometry->GetIndexToWorldTransform() );
   //outputImageGeometry->SetBounds( boBoxRelativeToImage->GetBounds() );
   //outputImageGeometry->Initialize();

   //outputImage->Initialize( inputMitkImage->GetPixelType(), *outputImageGeometry, false );

  // PART I: initialize input requested region. We do this already here (and not 
  // later when GenerateInputRequestedRegion() is called), because we 
  // also need the information to setup the output.
  
  mitk::BoundingBox::PointType min = boBoxRelativeToImage->GetMinimum();
  mitk::BoundingBox::PointType max = boBoxRelativeToImage->GetMaximum();
  
  m_InputRequestedRegion = input->GetLargestPossibleRegion();

  typename mitk::SlicedData::IndexType start = m_InputRequestedRegion.GetIndex();
  if(min[0]<0.0) min[0]=0.0;
  if(min[1]<0.0) min[1]=0.0;
  if(min[2]<0.0) min[2]=0.0;
  start[0] = min[0] + 0.5; //to avoid rounding errors
  start[1] = min[1] + 0.5;
  start[2] = min[2] + 0.5;
  m_InputRequestedRegion.SetIndex(start);
  
  typename mitk::SlicedData::SizeType size = m_InputRequestedRegion.GetSize();
  //the largest possible reagion of an image always starts at 0,
  //thus it's ok to compare (0+)size[i] with max[i] 
  if ( size[0] < max[0] )
    size[0] -= start[0];
  else 
    size[0] = max[0] - start[0];
  if ( size[1] < max[1] )
    size[1] -= start[1];
  else 
    size[1] = max[1] - start[1];
  if ( size[2] < max[2] )
    size[2] -= start[2];
  else 
    size[2] = max[2] - start[2];
  m_InputRequestedRegion.SetSize(size);

  // PART II: initialize output image
  
  // Currently working for 3D images only
  unsigned int dim[3];
  itk2vtk(size, dim);
  output->Initialize(mitk::PixelType(typeid(TPixel)), 3, dim);

  // set the spacing
  mitk::Vector3D spacing = input->GetSlicedGeometry()->GetSpacing();
  output->SetSpacing(spacing);
  
  // Position the output Image to match the corresponding region of the input image
  mitk::Geometry3D* outputGeometry = output->GetGeometry();
  outputGeometry->GetVtkTransform()->Translate(start[0], start[1], start[2]);
  outputGeometry->TransferVtkToITKTransform();

  m_TimeOfHeaderInitialization.Modified();
}

template <typename TPixel>
void BoundingObjectCutter<TPixel>::GenerateData() 
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if(input.IsNull())
    return;  

  if(m_BoundingObject.IsNull())
    return;

  // PART 1: convert input MITK image to ITK image
  typename ItkImageType::Pointer inputItkImage;
  CastToItkImage(input, inputItkImage);

  if (inputItkImage.IsNull())
  {
    mitk::StatusBar::DisplayErrorText ("An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
    std::cout << " image is NULL...returning" << std::endl;
    return; 
  }

  // PART 2: convert m_InputRequestedRegion (type mitk::SlicedData::RegionType)
  // into ITK-image-region (ItkImageType::RegionType)
  // unfortunately, we cannot use input->GetRequestedRegion(), because it
  // has been destroyed by the mitk::CastToItkImage call of PART 1
  // (which sets the m_RequestedRegion to the LargestPossibleRegion).
  // Thus, use our own member m_InputRequestedRegion insead.
  
  // first convert the index
  typename ItkImageType::IndexType::IndexValueType tmpIndex[3];
  itk2vtk(m_InputRequestedRegion.GetIndex(), tmpIndex);
  typename ItkImageType::IndexType index;
  index.SetIndex(tmpIndex);
  
  // then convert the size
  typename ItkImageType::SizeType::SizeValueType tmpSize[3];
  itk2vtk(m_InputRequestedRegion.GetSize(), tmpSize);
  typename ItkImageType::SizeType size;
  size.SetSize(tmpSize);
  
  //create the ITK-image-region out of index and size
  ItkRegionType inputRegionOfInterest(index, size);

  // PART 3: get access to the MITK output image via an ITK image
  mitk::ImageToItk<typename TPixel, 3>::Pointer outputimagetoitk = mitk::ImageToItk<typename TPixel, 3>::New();
  outputimagetoitk->SetInput(output);
  outputimagetoitk->Update();
  typename ItkImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

  // PART 4: iterate over input and output using ITK iterators
  
  // create the iterators
  ItkImageIteratorType inputIt( inputItkImage, inputRegionOfInterest );
  ItkImageIteratorType outputIt( outputItkImage, outputItkImage->GetLargestPossibleRegion() );

  // Cut the boundingbox out of the image by iterating through 
  // all pixels and checking if they are inside using IsInside()
  m_OutsidePixelCount = 0;
  m_InsidePixelCount = 0;
  mitk::Point3D p;
  mitk::Geometry3D* inputGeometry = input->GetGeometry();

  //shall we use a fixed value for each inside pixel?
  if (GetUseInsideValue())
  {
    // yes, use a fixed value for each inside pixel (create a binary mask of the bounding object)
    for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
    {
      vtk2itk(inputIt.GetIndex(), p);
      inputGeometry->UnitsToMM(p, p);
      if(m_BoundingObject->IsInside(p))
      {
        outputIt.Set(m_InsideValue);
        ++m_InsidePixelCount;
      }
      else
      {
        outputIt.Set(m_OutsideValue);
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
      inputGeometry->UnitsToMM(p, p);
      if(m_BoundingObject->IsInside(p))
      {
        outputIt.Set( inputIt.Value() );
        ++m_InsidePixelCount;
      }
      else
      {
        outputIt.Set( m_OutsideValue );
        ++m_OutsidePixelCount;
      }
    }
  }
  m_TimeOfHeaderInitialization.Modified();
}

} // of namespace mitk
#endif // of MITKBOUNDINGOBJECTCUTTER_TXX
