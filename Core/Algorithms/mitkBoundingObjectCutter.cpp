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

#include "mitkBoundingObjectCutter.h"
#include "mitkTimeHelper.h"
#include "mitkImageAccessByItk.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"

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
  m_InputTimeSelector  = mitk::ImageTimeSelector::New();
  m_OutputTimeSelector = mitk::ImageTimeSelector::New();
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
  // we have already calculated the spatial part of the 
  // input-requested-region in m_InputRequestedRegion in 
  // GenerateOutputInformation (which is called before 
  // GenerateInputRequestedRegion).
  GenerateTimeInInputRegion(this->GetOutput(), const_cast< mitk::Image * > ( this->GetInput() ));
  GenerateTimeInInputRegion(this->GetOutput(), m_BoundingObject.GetPointer());
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

  if((m_BoundingObject.IsNull()) || (m_BoundingObject->GetTimeSlicedGeometry()->GetTimeSteps() == 0))
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

  // pre-initialize input-requested-region to largest-possible-region
  // and correct time-region; spatial part will be cropped by 
  // bounding-box of bounding-object below
  m_InputRequestedRegion = input->GetLargestPossibleRegion();

  // build region out of bounding-box of bounding-object
  mitk::SlicedData::IndexType index=m_InputRequestedRegion.GetIndex(); //init times and channels
  vtk2itk(boBoxRelativeToImage->GetMinimum(), index);

  mitk::SlicedData::SizeType  size = m_InputRequestedRegion.GetSize(); //init times and channels
  mitk::BoundingBox::PointType max = boBoxRelativeToImage->GetMaximum();
  size[0]=max[0]-index[0];
  size[1]=max[1]-index[1];
  size[2]=max[2]-index[2];

  mitk::SlicedData::RegionType boRegion(index, size);
  
  // crop input-requested-region with region of bounding-object
  if(m_InputRequestedRegion.Crop(boRegion)==false)
  {
    // crop not possible => do nothing: set time size to 0.
    size.Fill(0);
    m_InputRequestedRegion.SetSize(size);
    boRegion.SetSize(size);
    m_BoundingObject->SetRequestedRegion(&boRegion);
    return;
  }

  // set input-requested-region, because we access it later in
  // GenerateInputRequestedRegion (there we just set the time)
  input->SetRequestedRegion(&m_InputRequestedRegion);

  // PART II: initialize output image
  
  unsigned int dimension = input->GetDimension();
  unsigned int *dimensions = new unsigned int [dimension];
  itk2vtk(m_InputRequestedRegion.GetSize(), dimensions);
  if(dimension>3)
    memcpy(dimensions+3, input->GetDimensions()+3, (dimension-3)*sizeof(unsigned int));
  output->Initialize(mitk::PixelType(GetOutputPixelType()), dimension, dimensions);
  delete [] dimensions;

  // set the spacing
  mitk::Vector3D spacing = input->GetSlicedGeometry()->GetSpacing();
  output->SetSpacing(spacing);
  
  // Position the output Image to match the corresponding region of the input image
  mitk::SlicedGeometry3D* slicedGeometry = output->GetSlicedGeometry();
  const mitk::SlicedData::IndexType& start = m_InputRequestedRegion.GetIndex();
  slicedGeometry->GetVtkTransform()->Translate(start[0], start[1], start[2]);
  slicedGeometry->TransferVtkToITKTransform();

  mitk::TimeSlicedGeometry* timeSlicedGeometry = output->GetTimeSlicedGeometry();
  timeSlicedGeometry->InitializeEvenlyTimed(slicedGeometry, output->GetDimension(3));
  timeSlicedGeometry->CopyTimes(input->GetTimeSlicedGeometry());

  m_TimeOfHeaderInitialization.Modified();
}

template < typename TPixel, unsigned int VImageDimension > 
void BoundingObjectCutter::CutImage( itk::Image< TPixel, VImageDimension >* inputItkImage, int boTimeStep, TPixel* dummy )
{
  CutImageWithOutputTypeSelect<TPixel, VImageDimension, TPixel>(inputItkImage, boTimeStep, dummy);
}

void BoundingObjectCutter::ComputeData(mitk::Image* input3D, int boTimeStep)
{
  AccessFixedDimensionByItk_1(input3D, CutImage, 3, boTimeStep);
}

void BoundingObjectCutter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if(input.IsNull())
    return;  

  if(m_BoundingObject.IsNull())
    return;

  m_InputTimeSelector->SetInput(input);
  m_OutputTimeSelector->SetInput(this->GetOutput());

  mitk::Surface::RegionType outputRegion = output->GetRequestedRegion();
  const mitk::TimeSlicedGeometry *outputTimeGeometry = output->GetTimeSlicedGeometry();
  const mitk::TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
  const mitk::TimeSlicedGeometry *boundingObjectTimeGeometry = m_BoundingObject->GetTimeSlicedGeometry();
  ScalarType timeInMS;

  int timestep=0;
  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3);

  int t;
  for(t=tstart;t<tmax;++t)
  {
    timeInMS = outputTimeGeometry->TimeStepToMS( t );

    timestep = inputTimeGeometry->MSToTimeStep( timeInMS );

    m_InputTimeSelector->SetTimeNr(timestep);
    m_InputTimeSelector->UpdateLargestPossibleRegion();
    m_OutputTimeSelector->SetTimeNr(timestep);
    m_OutputTimeSelector->UpdateLargestPossibleRegion();

    timestep = boundingObjectTimeGeometry->MSToTimeStep( timeInMS );

    ComputeData(m_InputTimeSelector->GetOutput(), timestep);
  }

  m_TimeOfHeaderInitialization.Modified();
}

} // of namespace mitk
