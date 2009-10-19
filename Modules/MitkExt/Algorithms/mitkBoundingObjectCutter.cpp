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

#if(_MSC_VER==1200)
#include <itkFixedCenterOfRotationAffineTransform.h>
#include <itkIndex.h>
#include <itkConceptChecking.h>
#endif
#include "mitkBoundingObjectCutter.h"
#include "mitkBoundingObjectCutter.txx"
#include "mitkTimeHelper.h"
#include "mitkImageAccessByItk.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"
//#include "itkImageRegionIteratorWithIndex.h"

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
  : m_BoundingObject(NULL), m_InsideValue(1), m_OutsideValue(0), m_AutoOutsideValue(false),
    m_UseInsideValue(false), m_OutsidePixelCount(0), m_InsidePixelCount(0), m_UseWholeInputRegion(false)
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
  mitk::Image* output = this->GetOutput();
  if((output->IsInitialized()==false) || (m_BoundingObject.IsNull()) || (m_BoundingObject->GetTimeSlicedGeometry()->GetTimeSteps() == 0))
    return;
  // we have already calculated the spatial part of the 
  // input-requested-region in m_InputRequestedRegion in 
  // GenerateOutputInformation (which is called before 
  // GenerateInputRequestedRegion).
  GenerateTimeInInputRegion(output, const_cast< mitk::Image * > ( this->GetInput() ));
  GenerateTimeInInputRegion(output, m_BoundingObject.GetPointer());
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
  mitk::SlicedData::IndexType  index=m_InputRequestedRegion.GetIndex(); //init times and channels
  mitk::BoundingBox::PointType min = boBoxRelativeToImage->GetMinimum();
  index[0] = (mitk::SlicedData::IndexType::IndexValueType)(min[0]+0.5);
  index[1] = (mitk::SlicedData::IndexType::IndexValueType)(min[1]+0.5);
  index[2] = (mitk::SlicedData::IndexType::IndexValueType)(min[2]+0.5);

  mitk::SlicedData::SizeType   size = m_InputRequestedRegion.GetSize(); //init times and channels
  mitk::BoundingBox::PointType max = boBoxRelativeToImage->GetMaximum();
  size[0] = (mitk::SlicedData::SizeType::SizeValueType)(max[0]+0.5)-index[0];
  size[1] = (mitk::SlicedData::SizeType::SizeValueType)(max[1]+0.5)-index[1];
  size[2] = (mitk::SlicedData::SizeType::SizeValueType)(max[2]+0.5)-index[2];

  mitk::SlicedData::RegionType boRegion(index, size);
  
  if(m_UseWholeInputRegion == false)
  {
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

  // now we have everything to initialize the transform of the output
  mitk::SlicedGeometry3D* slicedGeometry = output->GetSlicedGeometry();

  // set the transform: use the transform of the input; 
  // the origin will be replaced afterwards
  AffineTransform3D::Pointer indexToWorldTransform = AffineTransform3D::New();
  indexToWorldTransform->SetParameters(input->GetSlicedGeometry()->GetIndexToWorldTransform()->GetParameters());
  slicedGeometry->SetIndexToWorldTransform(indexToWorldTransform);
  
  // Position the output Image to match the corresponding region of the input image
  const mitk::SlicedData::IndexType& start = m_InputRequestedRegion.GetIndex();
  mitk::Point3D origin; vtk2itk(start, origin);
  inputImageGeometry->IndexToWorld(origin, origin);
  slicedGeometry->SetOrigin(origin);

  mitk::TimeSlicedGeometry* timeSlicedGeometry = output->GetTimeSlicedGeometry();
  timeSlicedGeometry->InitializeEvenlyTimed(slicedGeometry, output->GetDimension(3));
  timeSlicedGeometry->CopyTimes(input->GetTimeSlicedGeometry());

  m_TimeOfHeaderInitialization.Modified();
}

void BoundingObjectCutter::ComputeData(mitk::Image* input3D, int boTimeStep)
{
  AccessFixedDimensionByItk_2(input3D, CutImage, 3, this, boTimeStep);
}

void BoundingObjectCutter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if(input.IsNull())
    return;  

  if((output->IsInitialized()==false) || (m_BoundingObject.IsNull()) || (m_BoundingObject->GetTimeSlicedGeometry()->GetTimeSteps() == 0))
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
    m_OutputTimeSelector->SetTimeNr(t);
    m_OutputTimeSelector->UpdateLargestPossibleRegion();

    timestep = boundingObjectTimeGeometry->MSToTimeStep( timeInMS );

    ComputeData(m_InputTimeSelector->GetOutput(), timestep);
  }

  m_TimeOfHeaderInitialization.Modified();
}

} // of namespace mitk
