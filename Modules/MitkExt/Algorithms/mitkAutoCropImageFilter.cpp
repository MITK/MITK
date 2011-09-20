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
#include "mitkImageAccessByItk.h"
#include "mitkGeometry3D.h"
#include "mitkStatusBar.h"

#include <itkImageRegionConstIterator.h>
#include <itkRegionOfInterestImageFilter.h>


mitk::AutoCropImageFilter::AutoCropImageFilter()
: m_BackgroundValue(0),
  m_MarginFactor(1.0),
  m_TimeSelector(NULL),
  m_OverrideCroppingRegion(false)
{

}


mitk::AutoCropImageFilter::~AutoCropImageFilter()
{

}


template < typename TPixel, unsigned int VImageDimension>
void mitk::AutoCropImageFilter::ITKCrop3DImage( itk::Image< TPixel, VImageDimension >* inputItkImage, unsigned int timestep)
{

  if (inputItkImage == NULL)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText ("An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
    MITK_ERROR << "image is NULL...returning" << std::endl;
    return;
  }

  typedef itk::Image< TPixel, VImageDimension > InternalImageType;
  typedef typename InternalImageType::Pointer   InternalImagePointer;

  typedef itk::RegionOfInterestImageFilter < InternalImageType, InternalImageType > ROIFilterType;
  typedef typename itk::RegionOfInterestImageFilter < InternalImageType, InternalImageType >::Pointer ROIFilterPointer; 

  InternalImagePointer outputItk = InternalImageType::New();

  ROIFilterPointer roiFilter = ROIFilterType::New();
  roiFilter->SetInput(0,inputItkImage);
  roiFilter->SetRegionOfInterest(this->GetCroppingRegion());
  roiFilter->Update();
  outputItk = roiFilter->GetOutput();
  outputItk->DisconnectPipeline();

  mitk::Image::Pointer newMitkImage = mitk::Image::New();
  mitk::CastToMitkImage( outputItk, newMitkImage );

  const mitk::ChannelDescriptor desc = newMitkImage->GetChannelDescriptor(0);
  unsigned char* image3D = desc.GetData();
  this->GetOutput()->SetVolume( (void*) &image3D , timestep );
}

void mitk::AutoCropImageFilter::GenerateOutputInformation()
{
  mitk::Image::Pointer input = const_cast<mitk::Image*> (this->GetInput());
  mitk::Image::Pointer output = this->GetOutput();

  if(input->GetDimension() <= 2)
  {
    MITK_ERROR << "Only 3D any 4D images are supported." << std::endl;
    return;
  }

  ComputeNewImageBounds();

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

  // build region out of index and size calculated in ComputeNewImageBounds()

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

  mitk::SlicedData::RegionType cropRegion(index, size);

  // crop input-requested-region with cropping region computed from the image data
  if(m_InputRequestedRegion.Crop(cropRegion)==false)
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

  
  //clone the IndexToWorldTransform from the input, otherwise we will overwrite it, when adjusting the origin of the output image!!
  itk::ScalableAffineTransform< mitk::ScalarType,3 >::Pointer cloneTransform = itk::ScalableAffineTransform< mitk::ScalarType,3 >::New();
  cloneTransform->Compose(input->GetGeometry()->GetIndexToWorldTransform());
  output->GetGeometry()->SetIndexToWorldTransform( cloneTransform.GetPointer() );

  // Set the spacing
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

  if(input->GetDimension() <= 2)
  {
    MITK_ERROR << "Only 3D and 4D images supported";
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
  mitk::Image::ConstPointer inputMitk = this->GetInput();

  if (m_OverrideCroppingRegion)
  {
    for (unsigned int i=0; i<3; ++i)
    {
      m_RegionIndex[i] = m_CroppingRegion.GetIndex()[i];
      m_RegionSize[i] = m_CroppingRegion.GetSize()[i];

      if (m_RegionIndex[i] >= inputMitk->GetDimension(i))
      {
        itkExceptionMacro("Cropping index is not inside the image. "
                       << std::endl << "Index:"
                       << std::endl << m_CroppingRegion.GetIndex()
                       << std::endl << "Size:"
                       << std::endl << m_CroppingRegion.GetSize());
      }

      if (m_RegionIndex[i] + m_RegionSize[i] >= inputMitk->GetDimension(i))
      {
        m_RegionSize[i] = inputMitk->GetDimension(i) - m_RegionIndex[i];
      }
    }

    for (unsigned int i=0; i<3; ++i)
    {
      m_RegionIndex[i] = m_CroppingRegion.GetIndex()[i];
      m_RegionSize[i] = m_CroppingRegion.GetSize()[i];

    }
  }
  else
  {
    // Check if a 3D or 4D image is present
    unsigned int timeSteps = 1;
    if (inputMitk->GetDimension() == 4 ) 
      timeSteps = inputMitk->GetDimension(3);

    ImageType::IndexType minima,maxima;

    if (inputMitk->GetDimension() == 4)
    {
      // initialize with time step 0
      m_TimeSelector = mitk::ImageTimeSelector::New();
      m_TimeSelector->SetInput( inputMitk );
      m_TimeSelector->SetTimeNr( 0 );
      m_TimeSelector->UpdateLargestPossibleRegion();
      inputMitk = m_TimeSelector->GetOutput();
    }

    ImagePointer inputItk = ImageType::New();
    mitk::CastToItkImage( inputMitk , inputItk );

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
        inputMitk = m_TimeSelector->GetOutput();
        mitk::CastToItkImage( inputMitk , inputItk );
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

    m_RegionSize[0] = (SizeValueType)(m_MarginFactor * (maxima[0] - minima[0] + 1 ));
    m_RegionSize[1] = (SizeValueType)(m_MarginFactor * (maxima[1] - minima[1] + 1 ));
    m_RegionSize[2] = (SizeValueType)(m_MarginFactor * (maxima[2] - minima[2] + 1 ));
    m_RegionIndex = minima;

    m_RegionIndex[0] -= (m_RegionSize[0] - maxima[0] + minima[0] - 1 )/2;
    m_RegionIndex[1] -= (m_RegionSize[1] - maxima[1] + minima[1] - 1 )/2;
    m_RegionIndex[2] -= (m_RegionSize[2] - maxima[2] + minima[2] - 1 )/2;

    ImageType::RegionType cropRegion(m_RegionIndex,m_RegionSize);
    origRegion.Crop(cropRegion);

    m_RegionSize[0] = origRegion.GetSize()[0];
    m_RegionSize[1] = origRegion.GetSize()[1];
    m_RegionSize[2] = origRegion.GetSize()[2];

    m_RegionIndex[0] = origRegion.GetIndex()[0];
    m_RegionIndex[1] = origRegion.GetIndex()[1];
    m_RegionIndex[2] = origRegion.GetIndex()[2];

    m_CroppingRegion = origRegion;
  }
}


void mitk::AutoCropImageFilter::GenerateInputRequestedRegion()
{

}

const mitk::PixelType mitk::AutoCropImageFilter::GetOutputPixelType()
{
  return this->GetInput()->GetPixelType();
}

void mitk::AutoCropImageFilter::SetCroppingRegion(RegionType overrideRegion)
{
  m_CroppingRegion = overrideRegion;
  m_OverrideCroppingRegion = true;
}
