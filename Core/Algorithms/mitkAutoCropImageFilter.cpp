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

#include "mitkAutoCropImageFilter.h"

#include "mitkImageCast.h"
#include <itkImageRegionConstIterator.h>
#include "mitkImageAccessByItk.h"
#include "mitkGeometry3D.h"
#include "mitkStatusBar.h"


template < typename TPixel, unsigned int VImageDimension>
void mitk::AutoCropImageFilter::ITKCrop3DImage( itk::Image< TPixel, VImageDimension >* inputItkImage, unsigned int timestep)
{

  if (inputItkImage == NULL)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText ("An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
    std::cout << " image is NULL...returning" << std::endl;
    return; 
  }

  typedef itk::Image< TPixel, VImageDimension > InternalImageType;
  typedef typename InternalImageType::Pointer            InternalImagePointer;

  typedef itk::CropImageFilter<InternalImageType,InternalImageType> FilterType;
  typedef typename itk::CropImageFilter<InternalImageType,InternalImageType>::Pointer FilterPointer;

  InternalImagePointer outputItk = InternalImageType::New();

  FilterPointer cropFilter = FilterType::New();
  cropFilter->SetLowerBoundaryCropSize( m_LowerBounds );
  cropFilter->SetUpperBoundaryCropSize( m_UpperBounds );
  cropFilter->SetInput( inputItkImage );
  cropFilter->Update();
  outputItk = cropFilter->GetOutput();
  outputItk->DisconnectPipeline();

  mitk::Image::Pointer newMitkImage = mitk::Image::New();
  mitk::CastToMitkImage( outputItk, newMitkImage );

  ipPicDescriptor* image3D = newMitkImage->GetVolumeData(0)->GetPicDescriptor();
  this->GetOutput()->SetPicVolume( image3D , timestep );
}


mitk::AutoCropImageFilter::AutoCropImageFilter() 
: m_BackgroundValue(0), 
  m_MarginFactor(1.0),
  m_OverrideCroppingRegion(false)
{
  m_TimeSelector = mitk::ImageTimeSelector::New();
}


mitk::AutoCropImageFilter::~AutoCropImageFilter()
{
}

void mitk::AutoCropImageFilter::GenerateOutputInformation()
{
  mitk::Image::Pointer input = const_cast< mitk::Image * > ( this->GetInput() );
  if(input->GetDimension() <= 2)
  {
    std::cout << "Only 3D any 4D images are supported." << std::endl;
    return;
  }
  ComputeNewImageBounds();
  //  mitk::Image::ConstPointer input  = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();
  if ((output->IsInitialized()) && (output->GetPipelineMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<<"GenerateOutputInformation()");
  
  // PART I: initialize input requested region. We do this already here (and not 
  // later when GenerateInputRequestedRegion() is called), because we 
  // also need the information to setup the output.

  // pre-initialize input-requested-region to largest-possible-region
  // and correct time-region; spatial part will be cropped by 
  // bounding-box of bounding-object below
  m_InputRequestedRegion = input->GetLargestPossibleRegion();

  // build region out of bounding-box of cropping region size
  mitk::SlicedData::IndexType index;
  index[0] = m_RegionIndex[0];
  index[1] = m_RegionIndex[1];
  index[2] = m_RegionIndex[2];
  index[3] = m_InputRequestedRegion.GetIndex()[3]; 
  index[4] = m_InputRequestedRegion.GetIndex()[4]; 
  mitk::SlicedData::SizeType  size;
  size[0] = m_RegionSize[0]; 
  size[1] = m_RegionSize[1]; 
  size[2] = m_RegionSize[2]; 
  size[3] = m_InputRequestedRegion.GetSize()[3]; 
  size[4] = m_InputRequestedRegion.GetSize()[4]; 

  mitk::SlicedData::RegionType boRegion(index, size);

  // crop input-requested-region with cropping region computed from the image data
  if(m_InputRequestedRegion.Crop(boRegion)==false)
  {
    // crop not possible => do nothing: set time size to 0.
    size.Fill(0);
    m_InputRequestedRegion.SetSize(size);
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
  output->Initialize(mitk::PixelType( GetOutputPixelType() ), dimension, dimensions);
  delete [] dimensions;

  // Set the spacing
  output->GetGeometry()->SetIndexToWorldTransform( input->GetGeometry()->GetIndexToWorldTransform() );
  output->SetSpacing( input->GetSlicedGeometry()->GetSpacing() );
  
  // Position the output Image to match the corresponding region of the input image
  mitk::SlicedGeometry3D* slicedGeometry = output->GetSlicedGeometry();
  const mitk::SlicedData::IndexType& start = m_InputRequestedRegion.GetIndex();
  mitk::Point3D origin; vtk2itk(start, origin);
  input->GetSlicedGeometry()->IndexToWorld(origin, origin);
  slicedGeometry->SetOrigin(origin);

  mitk::TimeSlicedGeometry* timeSlicedGeometry = output->GetTimeSlicedGeometry();
  timeSlicedGeometry->InitializeEvenlyTimed(slicedGeometry, output->GetDimension(3));
  timeSlicedGeometry->CopyTimes(input->GetTimeSlicedGeometry());

  m_TimeOfHeaderInitialization.Modified();

  output->SetPropertyList(input->GetPropertyList()->Clone());
}

void mitk::AutoCropImageFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if(input.IsNull())
    return;  

  // TODO also implement 2D
  if(input->GetDimension() <= 2)
  {
    std::cerr << "Only 3D and 4D images supported";
    return;
  }

  if((output->IsInitialized()==false) )
    return;

  if( m_TimeSelector.IsNull() ) m_TimeSelector = mitk::ImageTimeSelector::New();

  m_TimeSelector->SetInput(input);

  mitk::SlicedData::RegionType outputRegion = input->GetRequestedRegion();

  int tstart = outputRegion.GetIndex(3);
  int tmax = tstart + outputRegion.GetSize(3);

  for( int timestep=tstart;timestep<tmax;++timestep )
  {
    m_TimeSelector->SetTimeNr(timestep);
    m_TimeSelector->UpdateLargestPossibleRegion();

    AccessFixedDimensionByItk_1( m_TimeSelector->GetOutput(), ITKCrop3DImage, 3, timestep );
  }

  // this->GetOutput()->Update(); // Not sure if this is necessary...

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::AutoCropImageFilter::ComputeNewImageBounds()
{
  mitk::Image::ConstPointer img = this->GetInput();

  if (m_OverrideCroppingRegion)
  {
    for (unsigned int i=0; i<3; ++i)
    {
      m_RegionIndex[i] = m_CroppingRegion.GetIndex()[i];
      m_RegionSize[i] = m_CroppingRegion.GetSize()[i];

      m_LowerBounds[i] = m_CroppingRegion.GetIndex()[i];
      m_UpperBounds[i] = img->GetDimension(i)-m_CroppingRegion.GetSize()[i]-m_LowerBounds[i];
    }
  }
  else
  {
    mitk::Image::Pointer inputMitk = const_cast< mitk::Image * > ( this->GetInput() );

    // Check if a 3D or 4D image is present
    unsigned int timeSteps = 1;
    if (inputMitk->GetDimension() == 4 ) timeSteps = inputMitk->GetDimension(3);

    ImageType::IndexType minima,maxima;

    m_TimeSelector = mitk::ImageTimeSelector::New();
    m_TimeSelector->SetInput( inputMitk );

    ImagePointer inputItk = ImageType::New();

    if (inputMitk->GetDimension() == 3) 
      img = inputMitk;
    else if (inputMitk->GetDimension() == 4)
    {
      // initialize with time step 0
      m_TimeSelector->SetTimeNr( 0 );
      m_TimeSelector->UpdateLargestPossibleRegion(); 
      img = m_TimeSelector->GetOutput();
    }

    mitk::CastToItkImage( img , inputItk );

    // it is assumed that all volumes in a time series have the same 3D dimensions
    ImageType::RegionType origRegion = inputItk->GetLargestPossibleRegion();

    // Initialize min and max on the first (or only) time step
    maxima = inputItk->GetLargestPossibleRegion().GetIndex();
    minima[0] = inputItk->GetLargestPossibleRegion().GetSize()[0];
    minima[1] = inputItk->GetLargestPossibleRegion().GetSize()[1];
    minima[2] = inputItk->GetLargestPossibleRegion().GetSize()[2];

    typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;

    for(unsigned int idx = 0; idx < timeSteps; ++idx)
    {
       // if 4D image, update time step and itk image
      if( idx > 0)
      {
        m_TimeSelector->SetTimeNr( idx );
        m_TimeSelector->UpdateLargestPossibleRegion(); 
        img = m_TimeSelector->GetOutput();
        mitk::CastToItkImage( img , inputItk );
        ImageType::RegionType xxx = inputItk->GetLargestPossibleRegion();
      }

      ConstIteratorType inIt( inputItk,  origRegion );

      for ( inIt.GoToBegin(); !inIt.IsAtEnd(); ++inIt)
      {
        float pix_val = inIt.Get();
        if ( fabs(pix_val - m_BackgroundValue) > mitk::eps )
        {
          for (int i=0; i < 3; i++)
          {
            minima[i] = vnl_math_min((int)minima[i],(int)(inIt.GetIndex()[i]));
            maxima[i] = vnl_math_max((int)maxima[i],(int)(inIt.GetIndex()[i]));
          }
        }
      }
    }

    typedef ImageType::RegionType::SizeType::SizeValueType  SizeValueType;

    m_RegionSize[0] = (SizeValueType)(m_MarginFactor * (maxima[0] - minima[0]));
    m_RegionSize[1] = (SizeValueType)(m_MarginFactor * (maxima[1] - minima[1]));
    m_RegionSize[2] = (SizeValueType)(m_MarginFactor * (maxima[2] - minima[2]));
    m_RegionIndex = minima;

    m_RegionIndex[0] -= (m_RegionSize[0] - maxima[0] + minima[0])/2;
    m_RegionIndex[1] -= (m_RegionSize[1] - maxima[1] + minima[1])/2;
    m_RegionIndex[2] -= (m_RegionSize[2] - maxima[2] + minima[2])/2;

    ImageType::RegionType cropRegion(m_RegionIndex,m_RegionSize);
    origRegion.Crop(cropRegion);
    
    m_RegionSize[0] = origRegion.GetSize()[0];
    m_RegionSize[1] = origRegion.GetSize()[1];
    m_RegionSize[2] = origRegion.GetSize()[2];

    m_RegionIndex[0] = origRegion.GetIndex()[0];
    m_RegionIndex[1] = origRegion.GetIndex()[1];
    m_RegionIndex[2] = origRegion.GetIndex()[2];

    m_LowerBounds[0] = m_RegionIndex[0];
    m_LowerBounds[1] = m_RegionIndex[1];
    m_LowerBounds[2] = m_RegionIndex[2];

    // It is assumed that the first and the last 3D images have the same LargestPossibleRegion.
    // If they have not, this will surely crash somewhere.
    m_UpperBounds[0] = inputItk->GetLargestPossibleRegion().GetSize()[0]-m_RegionSize[0]-m_RegionIndex[0];
    m_UpperBounds[1] = inputItk->GetLargestPossibleRegion().GetSize()[1]-m_RegionSize[1]-m_RegionIndex[1];
    m_UpperBounds[2] = inputItk->GetLargestPossibleRegion().GetSize()[2]-m_RegionSize[2]-m_RegionIndex[2];
  }
}


void mitk::AutoCropImageFilter::GenerateInputRequestedRegion()
{

}

const std::type_info& mitk::AutoCropImageFilter::GetOutputPixelType() 
{
  return *this->GetInput()->GetPixelType().GetTypeId();
}
  
void mitk::AutoCropImageFilter::SetCroppingRegion(RegionType overrideRegion)
{
  m_CroppingRegion = overrideRegion;
  m_OverrideCroppingRegion = true;
}
