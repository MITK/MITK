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
#include <itkImageRegionIterator.h>
#include <mitkImage.h>
#include <vtkLinearTransform.h>
#include "mitkTimeHelper.h"
#include "mitkImageAccessByItk.h"
#include "mitkGeometry3D.h"
#include "mitkStatusBar.h"


template < typename TPixel, unsigned int VImageDimension>
void mitk::AutoCropImageFilter::ITKCrop3DImage( itk::Image< TPixel, VImageDimension >* inputItkImage)
{
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


  // ************************* MITK import

  typedef typename itk::ImageBase<VImageDimension>::RegionType ItkRegionType;
  typedef itk::ImageRegionIteratorWithIndex< InternalImageType > ItkInputImageIteratorType;

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
  typename ItkRegionType::IndexType index;
  index.Fill(0);
  typename ItkRegionType::SizeType size;
  size[0] = outputItk->GetLargestPossibleRegion().GetSize()[0];
  size[1] = outputItk->GetLargestPossibleRegion().GetSize()[1];
  size[2] = outputItk->GetLargestPossibleRegion().GetSize()[2];
  
  //create the ITK-image-region out of index and size
  ItkRegionType inputRegionOfInterest(index, size);

  // PART 2: get access to the MITK output image via an ITK image
  typename mitk::ImageToItk<InternalImageType>::Pointer outputimagetoitk = mitk::ImageToItk<InternalImageType>::New();
  mitk::Image::Pointer timeImage = m_OutputTimeSelector->GetOutput();
  outputimagetoitk->SetInput(timeImage);
  outputimagetoitk->Update();
  typename InternalImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();
//  outputItkImage->DisconnectPipeline();

  // PART 3: iterate over input and output using ITK iterators
  
  // create the iterators
  ItkInputImageIteratorType  inputIt( inputItkImage, outputItk->GetLargestPossibleRegion());
  ItkInputImageIteratorType outputIt( outputItkImage, inputRegionOfInterest );

  // Cut the boundingbox out of the image by iterating through 
  // all pixels and checking if they are inside using IsInside()
  mitk::Point3D p;

  for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt, ++outputIt)
  {
    outputIt.Set(inputIt.Get());
  }

  // *************************
}


mitk::AutoCropImageFilter::AutoCropImageFilter() 
: m_BackgroundValue(0), 
  m_MarginFactor(1.0),
  m_OverrideCroppingRegion(false)
{
  m_InputTimeSelector  = mitk::ImageTimeSelector::New();
  m_OutputTimeSelector = mitk::ImageTimeSelector::New();
}


mitk::AutoCropImageFilter::~AutoCropImageFilter()
{
}

void mitk::AutoCropImageFilter::GenerateOutputInformation()
{
  ComputeNewImageBounds();
//  mitk::Image::ConstPointer input  = this->GetInput();
  mitk::Image::Pointer input = const_cast< mitk::Image * > ( this->GetInput() );
  mitk::Image::Pointer output = this->GetOutput();
  if ((output->IsInitialized()) && (output->GetPipelineMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<<"GenerateOutputInformation()");

  mitk::Geometry3D* inputImageGeometry = input->GetSlicedGeometry();
  
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
    boRegion.SetSize(size);
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

  // set the spacing
  mitk::Vector3D spacing = input->GetSlicedGeometry()->GetSpacing();
  output->SetSpacing(spacing);
  
  // Position the output Image to match the corresponding region of the input image
  mitk::SlicedGeometry3D* slicedGeometry = output->GetSlicedGeometry();
  const mitk::SlicedData::IndexType& start = m_InputRequestedRegion.GetIndex();
  mitk::Point3D origin; vtk2itk(start, origin);
  inputImageGeometry->IndexToWorld(origin, origin);
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

  if((output->IsInitialized()==false) )
    return;

  m_InputTimeSelector->SetInput(input);
  m_OutputTimeSelector->SetInput(this->GetOutput());

  mitk::SlicedData::RegionType outputRegion = input->GetRequestedRegion();
  const mitk::TimeSlicedGeometry *outputTimeGeometry = output->GetTimeSlicedGeometry();
  const mitk::TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();

  ScalarType timeInMS;

  int timestep=0;
  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3);

  int t;
  for(t=tstart;t<tmax;++t)
  {
    timeInMS = outputTimeGeometry->TimeStepToMS( t );

    timestep = inputTimeGeometry->MSToTimeStep( timeInMS );

    m_InputTimeSelector->SetTimeNr(t);
    m_InputTimeSelector->UpdateLargestPossibleRegion();
    m_OutputTimeSelector->SetTimeNr(t);
    m_OutputTimeSelector->UpdateLargestPossibleRegion();

    timestep = inputTimeGeometry->MSToTimeStep( timeInMS );

    // TODO what about 2D?
    AccessFixedDimensionByItk( m_InputTimeSelector->GetOutput(), ITKCrop3DImage, 3);
  }

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::AutoCropImageFilter::ComputeNewImageBounds()
{
  mitk::Image::ConstPointer img;
  if ( this->GetInput()->GetDimension()==4)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput( this->GetInput() );
    timeSelector->SetTimeNr( 0 );
    timeSelector->UpdateLargestPossibleRegion(); 
    img = timeSelector->GetOutput();
  }
  else if (this->GetInput()->GetDimension() == 3 )
  {
    img =  this->GetInput();
  }

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

    ImagePointer inputItk = ImageType::New();
    mitk::CastToItkImage( img , inputItk );
    typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;
    ConstIteratorType inIt( inputItk,  inputItk->GetLargestPossibleRegion() );
    ImageType::IndexType minima,maxima;
    maxima = inputItk->GetLargestPossibleRegion().GetIndex();
    minima[0] = inputItk->GetLargestPossibleRegion().GetSize()[0];
    minima[1] = inputItk->GetLargestPossibleRegion().GetSize()[1];
    minima[2] = inputItk->GetLargestPossibleRegion().GetSize()[2];

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

    m_RegionSize[0] = (ImageType::RegionType::SizeType::SizeValueType)(m_MarginFactor * (maxima[0] - minima[0]));
    m_RegionSize[1] = (ImageType::RegionType::SizeType::SizeValueType)(m_MarginFactor * (maxima[1] - minima[1]));
    m_RegionSize[2] = (ImageType::RegionType::SizeType::SizeValueType)(m_MarginFactor * (maxima[2] - minima[2]));
    m_RegionIndex = minima;

    m_RegionIndex[0] -= (m_RegionSize[0] - maxima[0] + minima[0])/2;
    m_RegionIndex[1] -= (m_RegionSize[1] - maxima[1] + minima[1])/2;
    m_RegionIndex[2] -= (m_RegionSize[2] - maxima[2] + minima[2])/2;

    ImageType::RegionType origRegion = inputItk->GetLargestPossibleRegion();
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

    m_UpperBounds[0] = inputItk->GetLargestPossibleRegion().GetSize()[0]-m_RegionSize[0]-m_RegionIndex[0];
    m_UpperBounds[1] = inputItk->GetLargestPossibleRegion().GetSize()[1]-m_RegionSize[1]-m_RegionIndex[1];
    m_UpperBounds[2] = inputItk->GetLargestPossibleRegion().GetSize()[2]-m_RegionSize[2]-m_RegionIndex[2];
  }
}


void mitk::AutoCropImageFilter::GenerateInputRequestedRegion()
{
  //todo
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
