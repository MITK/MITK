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

#include "mitkSegmentationInterpolationController.h"

#include "mitkImageCast.h"
#include "mitkExtractImageFilter.h"
#include "mitkImageTimeSelector.h"

#include "mitkShapeBasedInterpolationAlgorithm.h"

#include <itkCommand.h>
#include <itkImage.h>
#include <itkImageSliceConstIteratorWithIndex.h>

mitk::SegmentationInterpolationController::InterpolatorMapType mitk::SegmentationInterpolationController::s_InterpolatorForImage; // static member initialization
    
mitk::SegmentationInterpolationController* mitk::SegmentationInterpolationController::InterpolatorForImage(const Image* image)
{
  InterpolatorMapType::iterator iter = s_InterpolatorForImage.find( image );
  if ( iter != s_InterpolatorForImage.end() )
  {
    return iter->second;
  }
  else
  {
    return NULL;
  }
}

mitk::SegmentationInterpolationController::SegmentationInterpolationController()
:m_BlockModified(false)
{
}

mitk::SegmentationInterpolationController::~SegmentationInterpolationController()
{
  // remove this from the list of interpolators
  for ( InterpolatorMapType::iterator iter = s_InterpolatorForImage.begin();
        iter != s_InterpolatorForImage.end();
        ++iter )
  {
    if (iter->second == this)
    {
      s_InterpolatorForImage.erase( iter );
      break;
    }
  }
}
    
void mitk::SegmentationInterpolationController::OnImageModified(const itk::EventObject&)
{
  if (!m_BlockModified  && m_Segmentation.IsNotNull() )
  {
    SetSegmentationVolume( m_Segmentation );
  }
}
    
void mitk::SegmentationInterpolationController::BlockModified(bool block)
{
  m_BlockModified = block;
}

void mitk::SegmentationInterpolationController::SetSegmentationVolume( const Image* segmentation )
{
  // clear old information (remove all time steps
  m_SegmentationCountInSlice.clear();

  // delete this from the list of interpolators
  InterpolatorMapType::iterator iter = s_InterpolatorForImage.find( segmentation );
  if ( iter != s_InterpolatorForImage.end() )
  {
    s_InterpolatorForImage.erase( iter );
  }

  if (!segmentation) return;
  if (segmentation->GetDimension() > 4 || segmentation->GetDimension() < 3) 
  {
    itkExceptionMacro("SegmentationInterpolationController needs a 3D-segmentation or 3D+t, not 2D.");
  }

  if (m_Segmentation != segmentation)
  {
    // observe Modified() event of image
    itk::ReceptorMemberCommand<SegmentationInterpolationController>::Pointer command = itk::ReceptorMemberCommand<SegmentationInterpolationController>::New();
    command->SetCallbackFunction( this, &SegmentationInterpolationController::OnImageModified );
    segmentation->AddObserver( itk::ModifiedEvent(), command );
  }

  m_Segmentation = segmentation;

  m_SegmentationCountInSlice.resize( m_Segmentation->GetTimeSteps() );
  for (unsigned int timeStep = 0; timeStep < m_Segmentation->GetTimeSteps(); ++timeStep)
  {
    m_SegmentationCountInSlice[timeStep].resize(3);
    for (unsigned int dim = 0; dim < 3; ++dim)
    {
      m_SegmentationCountInSlice[timeStep][dim].clear();
      m_SegmentationCountInSlice[timeStep][dim].resize( m_Segmentation->GetDimension(dim) );
      m_SegmentationCountInSlice[timeStep][dim].assign( m_Segmentation->GetDimension(dim), 0 );
    }
  }

  s_InterpolatorForImage.insert( std::make_pair( m_Segmentation, this ) );

  // for all timesteps
  // scan whole image
  for (unsigned int timeStep = 0; timeStep < m_Segmentation->GetTimeSteps(); ++timeStep)
  {
    ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
    timeSelector->SetInput( m_Segmentation );
    timeSelector->SetTimeNr( timeStep );
    timeSelector->UpdateLargestPossibleRegion();
    Image::Pointer segmentation3D = timeSelector->GetOutput();
    AccessFixedDimensionByItk_2( segmentation3D, ScanWholeVolume, 3, m_Segmentation, timeStep );
  }

  //PrintStatus();
  
  SetReferenceVolume( m_ReferenceImage );

  Modified();
}

void mitk::SegmentationInterpolationController::SetReferenceVolume( const Image* referenceImage )
{
  m_ReferenceImage = referenceImage;

  if ( m_ReferenceImage.IsNull() ) return; // no image set - ignore it then
  assert ( m_Segmentation.IsNotNull() ); // should never happen

  // ensure the reference image has the same dimensionality and extents as the segmentation image
  if (    m_ReferenceImage.IsNull() 
       || m_Segmentation.IsNull()
       || m_ReferenceImage->GetDimension() != m_Segmentation->GetDimension() 
       || m_ReferenceImage->GetPixelType().GetNumberOfComponents() != 1
       || m_Segmentation->GetPixelType().GetNumberOfComponents() != 1
     )
  {
    std::cerr << __FILE__ << " l." << __LINE__ << ": original patient image does not match segmentation, ignoring patient image" << std::endl;
    m_ReferenceImage = NULL;
    return;
  }

  for (unsigned int dim = 0; dim < m_Segmentation->GetDimension(); ++dim)
    if ( m_ReferenceImage->GetDimension(dim) != m_Segmentation->GetDimension(dim) )
    {
      std::cerr << __FILE__ << " l." << __LINE__ 
                << ": original patient image does not match segmentation (different extent in dimension " << dim 
                << "), ignoring patient image" << std::endl;
      m_ReferenceImage = NULL;
      return;
    }
}
 
void mitk::SegmentationInterpolationController::SetChangedVolume( const Image* sliceDiff, unsigned int timeStep )
{
  if ( !sliceDiff ) return;
  if ( sliceDiff->GetDimension() != 3 ) return;
  
  AccessFixedDimensionByItk_1( sliceDiff, ScanChangedVolume, 3, timeStep );

  //PrintStatus();
  Modified();
}


void mitk::SegmentationInterpolationController::SetChangedSlice( const Image* sliceDiff, unsigned int sliceDimension, unsigned int sliceIndex, unsigned int timeStep )
{
  if ( !sliceDiff ) return;
  if ( sliceDimension > 2 ) return;
  if ( timeStep >= m_SegmentationCountInSlice.size() ) return;
  if ( sliceIndex >= m_SegmentationCountInSlice[timeStep][sliceDimension].size() ) return;

  unsigned int dim0(0);
  unsigned int dim1(1);

  // determine the other two dimensions
  switch (sliceDimension)
  {
    default:
    case 2:
      dim0 = 0; dim1 = 1; break;
    case 1:
      dim0 = 0; dim1 = 2; break;
    case 0:
      dim0 = 1; dim1 = 2; break;
  }

  ipPicDescriptor* rawSlice = const_cast<Image*>(sliceDiff)->GetSliceData()->GetPicDescriptor(); // we promise not to change anything!
  if (!rawSlice) return;

  AccessFixedDimensionByItk_1( sliceDiff, ScanChangedSlice, 2, SetChangedSliceOptions(sliceDimension, sliceIndex, dim0, dim1, timeStep, rawSlice->data) );
  
  //PrintStatus();
  
  Modified();
}

template < typename DATATYPE >
void mitk::SegmentationInterpolationController::ScanChangedSlice( itk::Image<DATATYPE, 2>*, const SetChangedSliceOptions& options )
{
  DATATYPE* pixelData( (DATATYPE*)options.pixelData );
  
  unsigned int timeStep( options.timeStep );

  unsigned int sliceDimension( options.sliceDimension );
  unsigned int sliceIndex( options.sliceIndex );

  if ( sliceDimension > 2 ) return;
  if ( sliceIndex >= m_SegmentationCountInSlice[timeStep][sliceDimension].size() ) return;
  
  unsigned int dim0( options.dim0 );
  unsigned int dim1( options.dim1 );

  int numberOfPixels(0); // number of pixels in this slice that are not 0

  unsigned int dim0max = m_SegmentationCountInSlice[timeStep][dim0].size();
  unsigned int dim1max = m_SegmentationCountInSlice[timeStep][dim1].size();

  // scan the slice from two directions
  // and set the flags for the two dimensions of the slice
  for (unsigned int v = 0; v < dim1max; ++v)
  {
    for (unsigned int u = 0; u < dim0max; ++u)
    {
      DATATYPE value = *(pixelData + u + v * dim0max);

      assert ( (signed) m_SegmentationCountInSlice[timeStep][dim0][u] + (signed)value >= 0 ); // just for debugging. This must always be true, otherwise some counting is going wrong
      assert ( (signed) m_SegmentationCountInSlice[timeStep][dim1][v] + (signed)value >= 0 );

      m_SegmentationCountInSlice[timeStep][dim0][u] = static_cast<unsigned int>( m_SegmentationCountInSlice[timeStep][dim0][u] + value ); 
      m_SegmentationCountInSlice[timeStep][dim1][v] = static_cast<unsigned int>( m_SegmentationCountInSlice[timeStep][dim1][v] + value );
      numberOfPixels += static_cast<int>( value );
    }
  }

  // flag for the dimension of the slice itself 
  assert ( (signed) m_SegmentationCountInSlice[timeStep][sliceDimension][sliceIndex] + numberOfPixels >= 0 );
  m_SegmentationCountInSlice[timeStep][sliceDimension][sliceIndex] += numberOfPixels;
  
  //std::cout << "scan t=" << timeStep << " from (0,0) to (" << dim0max << "," << dim1max << ") (" << pixelData << "-" << pixelData+dim0max*dim1max-1 <<  ") in slice " << sliceIndex << " found " << numberOfPixels << " pixels" << std::endl;
}


template < typename TPixel, unsigned int VImageDimension >
void mitk::SegmentationInterpolationController::ScanChangedVolume( itk::Image<TPixel, VImageDimension>* diffImage, unsigned int timeStep )
{
  typedef itk::ImageSliceConstIteratorWithIndex< itk::Image<TPixel, VImageDimension> > IteratorType;

  IteratorType iter( diffImage, diffImage->GetLargestPossibleRegion() );
  iter.SetFirstDirection(0);
  iter.SetSecondDirection(1);

  int numberOfPixels(0); // number of pixels in this slice that are not 0
  
  typename IteratorType::IndexType index;
  unsigned int x = 0;
  unsigned int y = 0;
  unsigned int z = 0;

  iter.GoToBegin();
  while ( !iter.IsAtEnd() )
  {
    while ( !iter.IsAtEndOfSlice() )
    {
      while ( !iter.IsAtEndOfLine() )
      {
        index = iter.GetIndex();

        x = index[0];
        y = index[1];
        z = index[2];

        TPixel value = iter.Get();

        assert ( (signed) m_SegmentationCountInSlice[timeStep][0][x] + (signed)value >= 0 ); // just for debugging. This must always be true, otherwise some counting is going wrong
        assert ( (signed) m_SegmentationCountInSlice[timeStep][1][y] + (signed)value >= 0 );

        m_SegmentationCountInSlice[timeStep][0][x] = static_cast<unsigned int>( m_SegmentationCountInSlice[timeStep][0][x] + value ); 
        m_SegmentationCountInSlice[timeStep][1][y] = static_cast<unsigned int>( m_SegmentationCountInSlice[timeStep][1][y] + value );
  
        numberOfPixels += static_cast<int>( value );
        
        ++iter;
      }
      iter.NextLine();
    }
    assert ( (signed) m_SegmentationCountInSlice[timeStep][2][z] + numberOfPixels >= 0 );
    m_SegmentationCountInSlice[timeStep][2][z] += numberOfPixels;
    numberOfPixels = 0;

    iter.NextSlice();
  }
}

    
template < typename DATATYPE >
void mitk::SegmentationInterpolationController::ScanWholeVolume( itk::Image<DATATYPE, 3>*, const Image* volume, unsigned int timeStep )
{
  if (!volume) return;
  if ( timeStep >= m_SegmentationCountInSlice.size() ) return;

  for (unsigned int slice = 0; slice < volume->GetDimension(2); ++slice)
  {
    DATATYPE* rawVolume = static_cast<DATATYPE*>( const_cast<Image*>(volume)->GetVolumeData(timeStep)->GetData() ); // we again promise not to change anything, we'll just count
    //DATATYPE* rawSlice = static_cast<DATATYPE*>( volume->GetSliceData(slice)->GetData() ); // TODO THIS wouldn't work. Did I mess up with some internal mitk::Image data structure?
    DATATYPE* rawSlice = rawVolume + ( volume->GetDimension(0) * volume->GetDimension(1) * slice );

    ScanChangedSlice<DATATYPE>( NULL, SetChangedSliceOptions(2, slice, 0, 1, timeStep, rawSlice) );
  }
}

void mitk::SegmentationInterpolationController::PrintStatus()
{
  unsigned int timeStep(0); // if needed, put a loop over time steps around everyting, but beware, output will be long

  std::cout << "Interpolator status (timestep 0): dimensions " 
            << m_SegmentationCountInSlice[timeStep][0].size() << " " 
            << m_SegmentationCountInSlice[timeStep][1].size() << " "
            << m_SegmentationCountInSlice[timeStep][2].size() << std::endl;
    
  std::cout << "Slice 0: " <<  m_SegmentationCountInSlice[timeStep][2][0] << std::endl;

  // row "x"
  for (unsigned int index = 0; index < m_SegmentationCountInSlice[timeStep][0].size(); ++index)
  {
    if ( m_SegmentationCountInSlice[timeStep][0][index] > 0 )
      std::cout << "O";
    else
      std::cout << ".";
  }
  std::cout << std::endl;
 
  // rows "y" and "z" (diagonal)
  for (unsigned int index = 1; index < m_SegmentationCountInSlice[timeStep][1].size(); ++index)
  {
    if ( m_SegmentationCountInSlice[timeStep][1][index] > 0 )
      std::cout << "O";
    else
      std::cout << ".";

    if ( m_SegmentationCountInSlice[timeStep][2].size() > index ) // if we also have a z value here, then print it, too
    {
      for (unsigned int indent = 1; indent < index; ++indent)
        std::cout << " ";

      if ( m_SegmentationCountInSlice[timeStep][2][index] > 0 )
        std::cout << m_SegmentationCountInSlice[timeStep][2][index];//"O";
      else
        std::cout << ".";
    }

    std::cout << std::endl;
  }

  // z indices that are larger than the biggest y index
  for (unsigned int index = m_SegmentationCountInSlice[timeStep][1].size(); index < m_SegmentationCountInSlice[timeStep][2].size(); ++index)
  {
    for (unsigned int indent = 0; indent < index; ++indent)
    std::cout << " ";

    if ( m_SegmentationCountInSlice[timeStep][2][index] > 0 )
      std::cout << m_SegmentationCountInSlice[timeStep][2][index];//"O";
    else
      std::cout << ".";
    
    std::cout << std::endl;
  }
}

mitk::Image::Pointer mitk::SegmentationInterpolationController::Interpolate( unsigned int sliceDimension, unsigned int sliceIndex, unsigned int timeStep )
{
  if (m_Segmentation.IsNull()) return NULL;

  if ( timeStep >= m_SegmentationCountInSlice.size() ) return NULL;
  if ( sliceDimension > 2 ) return NULL;
  unsigned int upperLimit = m_SegmentationCountInSlice[timeStep][sliceDimension].size();
  if ( sliceIndex >= upperLimit - 1 ) return NULL; // can't interpolate first and last slice
  if ( sliceIndex < 1  ) return NULL;

  if ( m_SegmentationCountInSlice[timeStep][sliceDimension][sliceIndex] > 0 ) return NULL; // slice contains a segmentation, won't interpolate anything then

  unsigned int lowerBound(0);
  unsigned int upperBound(0);
  bool bounds( false );

  for (lowerBound = sliceIndex - 1; /*lowerBound >= 0*/; --lowerBound)
  {
    if ( m_SegmentationCountInSlice[timeStep][sliceDimension][lowerBound] > 0 )
    {
      bounds = true;
      break;
    }

    if (lowerBound == 0) break; // otherwise overflow and start at something like 4294967295
  }

  if (!bounds) return NULL;

  bounds = false;
  for (upperBound = sliceIndex + 1 ; upperBound < upperLimit; ++upperBound)
  {
    if ( m_SegmentationCountInSlice[timeStep][sliceDimension][upperBound] > 0 )
    {
      bounds = true;
      break;
    }
  }

  if (!bounds) return NULL;
 
  // ok, we have found two neighboring slices with segmentations (and we made sure that the current slice does NOT contain anything
  //std::cout << "Interpolate in timestep " << timeStep << ", dimension " << sliceDimension << ": estimate slice " << sliceIndex << " from slices " << lowerBound << " and " << upperBound << std::endl;

  mitk::Image::Pointer lowerMITKSlice;
  mitk::Image::Pointer upperMITKSlice;
  mitk::Image::Pointer resultImage;
  try
  {
  // extract the two neighoring slices from the segmentation volume
    ExtractImageFilter::Pointer extractor= ExtractImageFilter::New();
    extractor->SetInput( m_Segmentation );
    extractor->SetSliceDimension( sliceDimension );
    extractor->SetSliceIndex( lowerBound );
    extractor->SetTimeStep( timeStep );
    extractor->Update();
    lowerMITKSlice = extractor->GetOutput();
    lowerMITKSlice->DisconnectPipeline(); // otherwise the next output of the filter will overwrite this pointer, too
    
    extractor->SetInput( m_Segmentation );
    extractor->SetSliceDimension( sliceDimension );
    extractor->SetSliceIndex( sliceIndex );
    extractor->SetTimeStep( timeStep );
    extractor->Update();
    resultImage = extractor->GetOutput();
    resultImage->DisconnectPipeline();
    
    extractor->SetInput( m_Segmentation );
    extractor->SetSliceDimension( sliceDimension );
    extractor->SetSliceIndex( upperBound );
    extractor->SetTimeStep( timeStep );
    extractor->Update();
    upperMITKSlice = extractor->GetOutput();

    if ( lowerMITKSlice.IsNull() || upperMITKSlice.IsNull() ) return NULL;
  }
  catch(...)
  {
    return NULL;
  }

  // interpolation algorithm gets some inputs
  //   two segmentations (guaranteed to be of the same data type, but no special data type guaranteed)
  //   orientation (sliceDimension) of the segmentations
  //   position of the two slices (sliceIndices)
  //   one volume image (original patient image)
  //
  // interpolation algorithm can use e.g. itk::ImageSliceConstIteratorWithIndex to
  //   inspect the original patient image at appropriate positions

  mitk::SegmentationInterpolationAlgorithm::Pointer algorithm = mitk::ShapeBasedInterpolationAlgorithm::New().GetPointer();
  return algorithm->Interpolate( lowerMITKSlice.GetPointer(), lowerBound,
                                 upperMITKSlice.GetPointer(), upperBound,
                                 sliceIndex,
                                 sliceDimension,
                                 resultImage,
                                 timeStep,
                                 m_ReferenceImage );

}

