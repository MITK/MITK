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

#ifndef MITKBOUNDINGOBJECTCUTTER_CPP
#define MITKBOUNDINGOBJECTCUTTER_CPP

#include "mitkBoundingObjectCutter.h"
#include "mitkImageAccessByItk.h"
#include "mitkImage.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"
#include "mitkImageToItk.h"
#include "mitkStatusBar.h"

#include <vtkTransform.h>

#include "mitkBoundingObjectCutter.txx"

namespace mitk
{

void BoundingObjectCutter::SetBoundingObject( const mitk::BoundingObject* boundingObject ) 
{
  m_BoundingObject = const_cast<mitk::BoundingObject*>(boundingObject);
	// Process object is not const-correct so the const_cast is required here
	this->ProcessObject::SetNthInput(1, 
		const_cast< mitk::BoundingObject * >( boundingObject ) );
}

const mitk::BoundingObject* BoundingObjectCutter::GetBoundingObject() const 
{
  return m_BoundingObject.GetPointer();
}

BoundingObjectCutter::BoundingObjectCutter() 
  : m_BoundingObject(NULL), m_InsideValue(1), m_OutsideValue(0), 
    m_UseInsideValue(false), m_OutsidePixelCount(0), m_InsidePixelCount(0)
{
  this->SetNumberOfInputs(2);
  this->SetNumberOfRequiredInputs(2);
}

BoundingObjectCutter::~BoundingObjectCutter()
{
}

const std::type_info& BoundingObjectCutter::GetOutputPixelType() 
{
  return *this->GetInput()->GetPixelType().GetTypeId();
}

void BoundingObjectCutter::GenerateInputRequestedRegion()
{
  // we have already calculated the m_InputRequestedRegion in 
  // GenerateOutputInformation (which is called before 
  // GenerateInputRequestedRegion)
  const_cast< mitk::Image * > ( this->GetInput() )->SetRequestedRegion(&m_InputRequestedRegion);
}

void BoundingObjectCutter::GenerateOutputInformation()
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

  // PART I: initialize input requested region. We do this already here (and not 
  // later when GenerateInputRequestedRegion() is called), because we 
  // also need the information to setup the output.
  
  mitk::BoundingBox::PointType min = boBoxRelativeToImage->GetMinimum();
  mitk::BoundingBox::PointType max = boBoxRelativeToImage->GetMaximum();
  
  m_InputRequestedRegion = input->GetLargestPossibleRegion();

  mitk::SlicedData::IndexType start = m_InputRequestedRegion.GetIndex();
  if(min[0]<0.0) min[0]=0.0;
  if(min[1]<0.0) min[1]=0.0;
  if(min[2]<0.0) min[2]=0.0;
  start[0] = min[0] + 0.5; //to avoid rounding errors
  start[1] = min[1] + 0.5;
  start[2] = min[2] + 0.5;
  m_InputRequestedRegion.SetIndex(start);
  
  mitk::SlicedData::SizeType size = m_InputRequestedRegion.GetSize();
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
  output->Initialize(mitk::PixelType(GetOutputPixelType()), 3, dim);

  // set the spacing
  mitk::Vector3D spacing = input->GetSlicedGeometry()->GetSpacing();
  output->SetSpacing(spacing);
  
  // Position the output Image to match the corresponding region of the input image
  mitk::Geometry3D* outputGeometry = output->GetGeometry();
  outputGeometry->GetVtkTransform()->Translate(start[0], start[1], start[2]);
  outputGeometry->TransferVtkToITKTransform();

  m_TimeOfHeaderInitialization.Modified();
}

template < typename TPixel, unsigned int VImageDimension > 
void BoundingObjectCutter::CutImage< TPixel, VImageDimension >( itk::Image< TPixel, VImageDimension >* inputItkImage, TPixel* dummy )
{
  CutImageWithOutputTypeSelect<TPixel, VImageDimension, TPixel>(inputItkImage, dummy);
}

void BoundingObjectCutter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();

  if(input.IsNull())
    return;  

  if(m_BoundingObject.IsNull())
    return;

  Access3DByItk(input, CutImage);

  m_TimeOfHeaderInitialization.Modified();
}

} // of namespace mitk
#endif // of MITKBOUNDINGOBJECTCUTTER_CPP

