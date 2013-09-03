/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkSegmentationInterpolationController.h"

#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include <mitkExtractSliceFilter.h>
#include "mitkImageReadAccessor.h"

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
: m_BlockModified(false), m_ActiveLabel(0), m_CurrentNumberOfLabels(0), m_WorkingImage(0), m_ReferenceImage(0)
{
}

void mitk::SegmentationInterpolationController::Activate2DInterpolation(bool status)
{
  m_2DInterpolationActivated = status;
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
/*
void mitk::SegmentationInterpolationController::OnWorkingImageModified(const itk::EventObject&)
{
  if (!m_BlockModified  && m_WorkingImage.IsNotNull() && m_2DInterpolationActivated )
  {
    this->Update();
  }
}
*/
void mitk::SegmentationInterpolationController::BlockModified(bool block)
{
  m_BlockModified = block;
}

void mitk::SegmentationInterpolationController::BuildLabelCount(void)
{
  if ( m_WorkingImage->GetNumberOfLabels() != m_CurrentNumberOfLabels )
  {
    m_LabelCountInSlice.clear();

    m_CurrentNumberOfLabels = m_WorkingImage->GetNumberOfLabels();

    m_LabelCountInSlice.resize( m_WorkingImage->GetTimeSteps() );

    for (unsigned int timeStep = 0; timeStep < m_WorkingImage->GetTimeSteps(); ++timeStep)
    {
      m_LabelCountInSlice[timeStep].resize(3);
      for (unsigned int dim = 0; dim < 3; ++dim)
      {
        m_LabelCountInSlice[timeStep][dim].clear();
        m_LabelCountInSlice[timeStep][dim].resize( m_WorkingImage->GetDimension(dim) );
        for (unsigned int slice = 0; slice < m_WorkingImage->GetDimension(dim); ++slice)
        {
          m_LabelCountInSlice[timeStep][dim][slice].clear();
          m_LabelCountInSlice[timeStep][dim][slice].resize( m_CurrentNumberOfLabels );
          m_LabelCountInSlice[timeStep][dim][slice].assign( m_CurrentNumberOfLabels, 0 );
        }
      }
    }

    // for all timesteps, scan whole image
    for (unsigned int timeStep = 0; timeStep < m_WorkingImage->GetTimeSteps(); ++timeStep)
    {
      ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
      timeSelector->SetInput( m_WorkingImage );
      timeSelector->SetTimeNr( timeStep );
      timeSelector->UpdateLargestPossibleRegion();
      Image::Pointer segmentation3D = timeSelector->GetOutput();
      AccessFixedDimensionByItk_1( segmentation3D, ScanChangedVolume, 3, timeStep );
    }
  }
}

void mitk::SegmentationInterpolationController::SetWorkingImage( LabelSetImage* newImage )
{
  if (!newImage) return;

  if (m_WorkingImage != newImage)
  {
    // delete this from the list of interpolators
    InterpolatorMapType::iterator iter = s_InterpolatorForImage.find( newImage );
    if ( iter != s_InterpolatorForImage.end() )
    {
      s_InterpolatorForImage.erase( iter );
    }
    m_WorkingImage = newImage;

    s_InterpolatorForImage.insert( std::make_pair( m_WorkingImage, this ) );
  }

  this->BuildLabelCount();

//  this->SetReferenceImage( m_ReferenceImage );

  this->Modified();
}

void mitk::SegmentationInterpolationController::SetReferenceImage( Image* newImage )
{
  if (!newImage) return;

  m_ReferenceImage = newImage;

  // ensure the reference image has the same dimensionality and extents as the segmentation image
  if (    m_ReferenceImage.IsNull()
       || m_WorkingImage.IsNull()
       || m_ReferenceImage->GetDimension() != m_WorkingImage->GetDimension()
       || m_ReferenceImage->GetPixelType().GetNumberOfComponents() != 1
       || m_WorkingImage->GetPixelType().GetNumberOfComponents() != 1
     )
  {
    MITK_WARN << "Segmentation image has different image characteristics than reference image." << std::endl;
    m_ReferenceImage = NULL;
    return;
  }

  for (unsigned int dim = 0; dim < m_WorkingImage->GetDimension(); ++dim)
    if ( m_ReferenceImage->GetDimension(dim) != m_WorkingImage->GetDimension(dim) )
    {
      MITK_WARN << "original patient image does not match segmentation (different extent in dimension " << dim
                << "), ignoring patient image" << std::endl;
      m_ReferenceImage = NULL;
      return;
    }
}

void mitk::SegmentationInterpolationController::SetChangedVolume( const Image* image, unsigned int timeStep )
{
  if ( !image ) return;
  if ( image->GetDimension() != 3 ) return;

  AccessFixedDimensionByItk_1( image, ScanChangedVolume, 3, timeStep );

  this->Modified();
}

void mitk::SegmentationInterpolationController::SetChangedSlice( const Image* slice, unsigned int sliceDimension, unsigned int sliceIndex, unsigned int timeStep )
{
  if ( !slice ) return;
  if ( sliceDimension > 2 ) return;
  if ( timeStep >= m_LabelCountInSlice.size() ) return;
  if ( sliceIndex >= m_LabelCountInSlice[timeStep][sliceDimension].size() ) return;

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

  AccessFixedDimensionByItk_1( slice, ScanChangedSlice, 2, SetChangedSliceOptions(sliceDimension, sliceIndex, dim0, dim1, timeStep) );

  this->Modified();
}

template < typename PixelType >
void mitk::SegmentationInterpolationController::ScanChangedSlice( itk::Image<PixelType, 2>* diffImage, const SetChangedSliceOptions& options )
{
//  PixelType* pixelData( (PixelType*)options.pixelData );

  unsigned int timeStep( options.timeStep );
  unsigned int sliceDimension( options.sliceDimension );
  unsigned int sliceIndex( options.sliceIndex );

  if ( sliceDimension > 2 ) return;
  if ( sliceIndex >= m_LabelCountInSlice[timeStep][sliceDimension].size() ) return;

  unsigned int dim0(options.dim0);
  unsigned int dim1(options.dim1);

  std::vector <int> numberOfPixels; // number of pixels in the current slice that are equal to the active label
  numberOfPixels.resize(m_WorkingImage->GetNumberOfLabels());

  typedef itk::Image<PixelType, 2> ImageType;
  typedef itk::ImageRegionConstIteratorWithIndex< ImageType > IteratorType;

  typename IteratorType iter( diffImage, diffImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  typename IteratorType::IndexType index;

  while ( !iter.IsAtEnd() )
  {
    index = iter.GetIndex();
    int value = static_cast< int >(iter.Get());
    ++ m_LabelCountInSlice[timeStep][dim0][index[0]][value];
    ++ m_LabelCountInSlice[timeStep][dim1][index[1]][value];
    ++ numberOfPixels[value];
    ++iter;
  }

  for (unsigned int label=0; label<m_WorkingImage->GetNumberOfLabels(); ++label)
  {
    m_LabelCountInSlice[timeStep][sliceDimension][sliceIndex][label] = numberOfPixels[label];
  }
}

/*
template < typename PixelType >
void mitk::SegmentationInterpolationController::ScanChangedSlice( itk::Image<PixelType, 2>*, const SetChangedSliceOptions& options )
{
  PixelType* pixelData( (PixelType*)options.pixelData );

  unsigned int timeStep( options.timeStep );
  unsigned int sliceDimension( options.sliceDimension );
  unsigned int sliceIndex( options.sliceIndex );

  if ( sliceDimension > 2 ) return;
  if ( sliceIndex >= m_LabelCountInSlice[timeStep][sliceDimension].size() ) return;

  unsigned int dim0( options.dim0 );
  unsigned int dim1( options.dim1 );

  std::vector <int> numberOfPixels; // number of pixels in the current slice that are equal to the active label
  numberOfPixels.resize(m_WorkingImage->GetNumberOfLabels());

  unsigned int dim0max = m_LabelCountInSlice[timeStep][dim0].size();
  unsigned int dim1max = m_LabelCountInSlice[timeStep][dim1].size();

  // scan the slice from two directions
  // and set the flags for the two dimensions of the slice
  for (unsigned int v = 0; v < dim1max; ++v)
  {
    for (unsigned int u = 0; u < dim0max; ++u)
    {
      int value = static_cast<int>(*(pixelData + u + v * dim0max));
      ++ m_LabelCountInSlice[timeStep][dim0][u][value];
      ++ m_LabelCountInSlice[timeStep][dim1][v][value];
      ++ numberOfPixels[value];
    }
  }

  for (unsigned int label = 0; label < m_WorkingImage->GetNumberOfLabels(); ++label)
  {
    m_LabelCountInSlice[timeStep][2][sliceIndex][label] = numberOfPixels[label];
  }
}
*/

template < typename TPixel, unsigned int VImageDimension >
void mitk::SegmentationInterpolationController::ScanChangedVolume( itk::Image<TPixel, VImageDimension>* diffImage, unsigned int timeStep )
{
  typedef itk::ImageSliceConstIteratorWithIndex< itk::Image<TPixel, VImageDimension> > IteratorType;

  IteratorType iter( diffImage, diffImage->GetLargestPossibleRegion() );
  iter.SetFirstDirection(0);
  iter.SetSecondDirection(1);

  typename IteratorType::IndexType index;
  unsigned int x = 0;
  unsigned int y = 0;
  unsigned int z = 0;

  std::vector <int> numberOfPixels; // number of pixels per slice that are equal to the active label
  numberOfPixels.resize(m_WorkingImage->GetNumberOfLabels());

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

        int value = static_cast<unsigned int>( iter.Get() );
        ++m_LabelCountInSlice[timeStep][0][x][value];
        ++m_LabelCountInSlice[timeStep][1][y][value];
        ++numberOfPixels[value];

        ++iter;
      }
      iter.NextLine();
    }

    for (unsigned int label = 0; label < m_WorkingImage->GetNumberOfLabels(); ++label)
      m_LabelCountInSlice[timeStep][2][z][label] += numberOfPixels[label];

    // clear per-slice label counter
    numberOfPixels.assign(m_WorkingImage->GetNumberOfLabels(), 0);
    iter.NextSlice();
  }
}

void mitk::SegmentationInterpolationController::PrintStatus()
{
  unsigned int timeStep(0); // if needed, put a loop over time steps around everyting, but beware, output will be long

  MITK_INFO << "Interpolator status (timestep 0): dimensions "
           << m_LabelCountInSlice[timeStep][0].size() << " "
           << m_LabelCountInSlice[timeStep][1].size() << " "
           << m_LabelCountInSlice[timeStep][2].size() << std::endl;

  MITK_INFO << "Slice 0: " <<  m_LabelCountInSlice[timeStep][2][0].size() << std::endl;
/*
  // row "x"
  for (unsigned int index = 0; index < m_LabelCountInSlice[timeStep][0].size(); ++index)
  {
    if ( m_LabelCountInSlice[timeStep][0][index] > 0 )
      MITK_INFO << "O";
    else
      MITK_INFO << ".";
  }
  MITK_INFO << std::endl;

  // rows "y" and "z" (diagonal)
  for (unsigned int index = 1; index < m_SegmentationCountInSlice[timeStep][1].size(); ++index)
  {
    if ( m_SegmentationCountInSlice[timeStep][1][index] > 0 )
      MITK_INFO << "O";
    else
      MITK_INFO << ".";

    if ( m_SegmentationCountInSlice[timeStep][2].size() > index ) // if we also have a z value here, then print it, too
    {
      for (unsigned int indent = 1; indent < index; ++indent)
        MITK_INFO << " ";

      if ( m_SegmentationCountInSlice[timeStep][2][index] > 0 )
        MITK_INFO << m_SegmentationCountInSlice[timeStep][2][index];//"O";
      else
        MITK_INFO << ".";
    }

    MITK_INFO << std::endl;
  }

  // z indices that are larger than the biggest y index
  for (unsigned int index = m_SegmentationCountInSlice[timeStep][1].size(); index < m_SegmentationCountInSlice[timeStep][2].size(); ++index)
  {
    for (unsigned int indent = 0; indent < index; ++indent)
    MITK_INFO << " ";

    if ( m_SegmentationCountInSlice[timeStep][2][index] > 0 )
      MITK_INFO << m_SegmentationCountInSlice[timeStep][2][index];//"O";
    else
      MITK_INFO << ".";

    MITK_INFO << std::endl;
  }
  */
}

mitk::Image::Pointer mitk::SegmentationInterpolationController::Interpolate(unsigned int sliceDimension,
                                                                            unsigned int sliceIndex,
                                                                            const mitk::PlaneGeometry* currentPlane,
                                                                            unsigned int timeStep )
{
  m_ActiveLabel = m_WorkingImage->GetActiveLabelIndex();

  if (m_WorkingImage.IsNull()) return NULL;
  if(!currentPlane)
  {
    return NULL;
  }
  if ( timeStep >= m_LabelCountInSlice.size() ) return NULL;
  if ( sliceDimension > 2 ) return NULL;
  unsigned int upperLimit = m_LabelCountInSlice[timeStep][sliceDimension].size();
  if ( sliceIndex >= upperLimit - 1 ) return NULL; // can't interpolate first and last slice
  if ( sliceIndex < 1  ) return NULL;

  // slice contains a segmentation, won't interpolate anything then
  if ( m_LabelCountInSlice[timeStep][sliceDimension][sliceIndex][m_ActiveLabel] > 0 ) return NULL;

  unsigned int lowerBound(0);
  unsigned int upperBound(0);
  bool bounds( false );

  for (lowerBound = sliceIndex - 1; /*lowerBound >= 0*/; --lowerBound)
  {
    if ( m_LabelCountInSlice[timeStep][sliceDimension][lowerBound][m_ActiveLabel] > 0 )
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
    if ( m_LabelCountInSlice[timeStep][sliceDimension][upperBound][m_ActiveLabel] > 0 )
    {
      bounds = true;
      break;
    }
  }

  if (!bounds) return NULL;

  // ok, we have found two neighboring slices with segmentations
  // (and we made sure that the current slice does NOT contain the active label
  //Setting up the ExtractSliceFilter
  mitk::ExtractSliceFilter::Pointer extractor = ExtractSliceFilter::New();
  extractor->SetInput(m_WorkingImage);
  extractor->SetTimeStep(timeStep);
  extractor->SetResliceTransformByGeometry( m_WorkingImage->GetTimeSlicedGeometry()->GetGeometry3D( timeStep ) );
  extractor->SetVtkOutputRequest(false);

  //Reslicing the current plane
  extractor->SetWorldGeometry(currentPlane);
  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch(const std::exception &e)
  {
    MITK_ERROR<<"Error in 2D interpolation: " << e.what();
    return NULL;
  }

  mitk::Image::Pointer resultImage = extractor->GetOutput();
  resultImage->DisconnectPipeline();

  //Creating PlaneGeometry for lower slice
  mitk::PlaneGeometry::Pointer reslicePlane = currentPlane->Clone();

  //Transforming the current origin so that it matches the lower slice
  mitk::Point3D origin = currentPlane->GetOrigin();
  m_WorkingImage->GetSlicedGeometry()->WorldToIndex(origin, origin);
  origin[sliceDimension] = lowerBound;
  m_WorkingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
  reslicePlane->SetOrigin(origin);

  //Extract the lower slice
  extractor->SetWorldGeometry(reslicePlane);
  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch(const std::exception &e)
  {
    MITK_ERROR<<"Error in 2D interpolation: " << e.what();
    return NULL;
  }

  mitk::Image::Pointer lowerMITKSlice = extractor->GetOutput();
  lowerMITKSlice->DisconnectPipeline();

  //Transforming the current origin so that it matches the upper slice
  m_WorkingImage->GetSlicedGeometry()->WorldToIndex(origin, origin);
  origin[sliceDimension] = upperBound;
  m_WorkingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
  reslicePlane->SetOrigin(origin);

  //Extract the upper slice
  extractor->SetWorldGeometry(reslicePlane);
  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch(const std::exception &e)
  {
    MITK_ERROR<<"Error in 2D interpolation: " << e.what();
    return NULL;
  }

  mitk::Image::Pointer upperMITKSlice = extractor->GetOutput();
  upperMITKSlice->DisconnectPipeline();

  if ( lowerMITKSlice.IsNull() || upperMITKSlice.IsNull() ) return NULL;

  // interpolation algorithm gets some inputs
  //   two segmentations (guaranteed to be of the same data type, but no special data type guaranteed)
  //   orientation (sliceDimension) of the segmentations
  //   position of the two slices (sliceIndices)
  //   one volume image (original patient image)
  //
  // interpolation algorithm can use e.g. itk::ImageSliceConstIteratorWithIndex to
  //   inspect the original patient image at appropriate positions

  mitk::SegmentationInterpolationAlgorithm::Pointer algorithm = mitk::ShapeBasedInterpolationAlgorithm::New();
  algorithm->Interpolate( lowerMITKSlice.GetPointer(), lowerBound,
                          upperMITKSlice.GetPointer(), upperBound,
                          sliceIndex,
                          sliceDimension,
                          resultImage,
                          timeStep,
                          m_ReferenceImage );

  return resultImage;
}
