/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSliceBasedInterpolationController.h"

#include "mitkExtractSliceFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageReadAccessor.h"
#include "mitkImageTimeSelector.h"

#include "mitkShapeBasedInterpolationAlgorithm.h"

#include <itkCommand.h>
#include <itkImage.h>
#include <itkImageSliceConstIteratorWithIndex.h>

mitk::SliceBasedInterpolationController::InterpolatorMapType
  mitk::SliceBasedInterpolationController::s_InterpolatorForImage; // static member initialization

mitk::SliceBasedInterpolationController *mitk::SliceBasedInterpolationController::InterpolatorForImage(
  const Image *image)
{
  auto iter = s_InterpolatorForImage.find(image);
  if (iter != s_InterpolatorForImage.end())
  {
    return iter->second;
  }
  else
  {
    return nullptr;
  }
}

mitk::SliceBasedInterpolationController::SliceBasedInterpolationController()
  : m_WorkingImage(nullptr), m_ReferenceImage(nullptr)
{
}

mitk::SliceBasedInterpolationController::~SliceBasedInterpolationController()
{
  // remove this from the list of interpolators
  for (auto iter = s_InterpolatorForImage.begin(); iter != s_InterpolatorForImage.end();
       ++iter)
  {
    if (iter->second == this)
    {
      s_InterpolatorForImage.erase(iter);
      break;
    }
  }
}

void mitk::SliceBasedInterpolationController::ResetLabelCount()
{
  m_LabelCountInSlice.clear();
  int numberOfLabels = m_WorkingImage->GetNumberOfLabels();
  m_LabelCountInSlice.resize(m_WorkingImage->GetTimeSteps());

  for (unsigned int timeStep = 0; timeStep < m_WorkingImage->GetTimeSteps(); ++timeStep)
  {
    m_LabelCountInSlice[timeStep].resize(3);
    for (unsigned int dim = 0; dim < 3; ++dim)
    {
      m_LabelCountInSlice[timeStep][dim].clear();
      m_LabelCountInSlice[timeStep][dim].resize(m_WorkingImage->GetDimension(dim));
      for (unsigned int slice = 0; slice < m_WorkingImage->GetDimension(dim); ++slice)
      {
        m_LabelCountInSlice[timeStep][dim][slice].clear();
        m_LabelCountInSlice[timeStep][dim][slice].resize(numberOfLabels);
        m_LabelCountInSlice[timeStep][dim][slice].assign(numberOfLabels, 0);
      }
    }
  }
}

void mitk::SliceBasedInterpolationController::SetWorkingImage(LabelSetImage *newImage)
{
  if (m_WorkingImage != newImage)
  {
    // delete the current working image from the list of interpolators
    auto iter = s_InterpolatorForImage.find(m_WorkingImage);
    if (iter != s_InterpolatorForImage.end())
    {
      s_InterpolatorForImage.erase(iter);
    }

    m_WorkingImage = newImage;

    s_InterpolatorForImage.insert(std::make_pair(m_WorkingImage, this));
  }

  this->ResetLabelCount();

  AccessFixedDimensionByItk_1(m_WorkingImage, ScanImageITKProcessing, 3, 0);

  // for all timesteps, scan whole image: TODO: enable this again for 3D+time
  /*
    for (unsigned int timeStep = 0; timeStep < m_WorkingImage->GetTimeSteps(); ++timeStep)
    {
      ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
      timeSelector->SetInput( m_WorkingImage );
      timeSelector->SetTimeNr( timeStep );
      timeSelector->UpdateLargestPossibleRegion();
      Image::Pointer segmentation3D = timeSelector->GetOutput();
      this->SetChangedVolume( segmentation3D.GetPointer(), timeStep );
    }
  */
  //   this->Modified();
}

void mitk::SliceBasedInterpolationController::SetReferenceImage(Image *newImage)
{
  if (!newImage)
    return;

  m_ReferenceImage = newImage;

  // ensure the reference image has the same dimensionality and extents as the segmentation image
  if (m_WorkingImage.IsNull() || m_ReferenceImage->GetDimension() != m_WorkingImage->GetDimension() ||
      m_ReferenceImage->GetPixelType().GetNumberOfComponents() != 1 ||
      m_WorkingImage->GetPixelType().GetNumberOfComponents() != 1)
  {
    MITK_WARN << "Segmentation image has different image characteristics than reference image." << std::endl;
    m_ReferenceImage = nullptr;
    return;
  }

  for (unsigned int dim = 0; dim < m_WorkingImage->GetDimension(); ++dim)
  {
    if (m_ReferenceImage->GetDimension(dim) != m_WorkingImage->GetDimension(dim))
    {
      MITK_WARN << "original patient image does not match segmentation (different extent in dimension " << dim
                << "), ignoring patient image" << std::endl;
      m_ReferenceImage = nullptr;
      return;
    }
  }
}

void mitk::SliceBasedInterpolationController::SetChangedSlice(const Image *slice,
                                                              unsigned int sliceDimension,
                                                              unsigned int sliceIndex,
                                                              unsigned int timeStep)
{
  if (!slice)
    return;
  if (slice->GetDimension() != 2)
    return;
  if (sliceDimension > 2)
    return;
  if (m_WorkingImage.IsNull())
    return;

  // check if the number of labels has changed
  auto numberOfLabels = m_WorkingImage->GetNumberOfLabels();
  if (m_LabelCountInSlice[0][0][0].size() != numberOfLabels)
    return;

  unsigned int dim0(0);
  unsigned int dim1(1);

  // determine the other two dimensions
  switch (sliceDimension)
  {
    default:
    case 2:
      dim0 = 0;
      dim1 = 1;
      break;
    case 1:
      dim0 = 0;
      dim1 = 2;
      break;
    case 0:
      dim0 = 1;
      dim1 = 2;
      break;
  }

  AccessFixedDimensionByItk_1(
    slice, ScanSliceITKProcessing, 2, SetChangedSliceOptions(sliceDimension, sliceIndex, dim0, dim1, timeStep));

  //  this->Modified();
}

template <typename PixelType>
void mitk::SliceBasedInterpolationController::ScanSliceITKProcessing(const itk::Image<PixelType, 2> *input,
                                                                     const SetChangedSliceOptions &options)
{
  unsigned int timeStep = options.timeStep;
  unsigned int sliceDimension = options.sliceDimension;
  unsigned int sliceIndex = options.sliceIndex;

  if (sliceDimension > 2)
    return;
  if (sliceIndex >= m_LabelCountInSlice[timeStep][sliceDimension].size())
    return;

  unsigned int dim0(options.dim0);
  unsigned int dim1(options.dim1);

  std::vector<int> numberOfPixels; // number of pixels in the current slice that are equal to the active label
  unsigned int numberOfLabels = m_WorkingImage->GetNumberOfLabels();
  numberOfPixels.resize(numberOfLabels);

  typedef itk::Image<PixelType, 2> ImageType;
  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> IteratorType;

  IteratorType iter(input, input->GetLargestPossibleRegion());
  iter.GoToBegin();

  typename IteratorType::IndexType index;

  while (!iter.IsAtEnd())
  {
    index = iter.GetIndex();
    auto value = static_cast<int>(iter.Get());
    ++m_LabelCountInSlice[timeStep][dim0][index[0]][value];
    ++m_LabelCountInSlice[timeStep][dim1][index[1]][value];
    ++numberOfPixels[value];
    ++iter;
  }

  for (unsigned int label = 0; label < numberOfLabels; ++label)
  {
    m_LabelCountInSlice[timeStep][sliceDimension][sliceIndex][label] = numberOfPixels[label];
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::SliceBasedInterpolationController::ScanImageITKProcessing(itk::Image<TPixel, VImageDimension> *input,
                                                                     unsigned int timeStep)
{
  typedef itk::ImageSliceConstIteratorWithIndex<itk::Image<TPixel, VImageDimension>> IteratorType;

  IteratorType iter(input, input->GetLargestPossibleRegion());
  iter.SetFirstDirection(0);
  iter.SetSecondDirection(1);

  typename IteratorType::IndexType index;
  unsigned int x = 0;
  unsigned int y = 0;
  unsigned int z = 0;

  std::vector<int> numberOfPixels; // number of pixels per slice that are equal to the active label
  unsigned int numberOfLabels = m_WorkingImage->GetNumberOfLabels();
  numberOfPixels.resize(numberOfLabels);

  iter.GoToBegin();
  while (!iter.IsAtEnd())
  {
    while (!iter.IsAtEndOfSlice())
    {
      while (!iter.IsAtEndOfLine())
      {
        index = iter.GetIndex();
        x = index[0];
        y = index[1];
        z = index[2];

        int value = static_cast<unsigned int>(iter.Get());
        ++m_LabelCountInSlice[timeStep][0][x][value];
        ++m_LabelCountInSlice[timeStep][1][y][value];
        ++numberOfPixels[value];

        ++iter;
      }
      iter.NextLine();
    }

    for (unsigned int label = 0; label < numberOfLabels; ++label)
      m_LabelCountInSlice[timeStep][2][z][label] += numberOfPixels[label];

    // clear label counter
    numberOfPixels.assign(numberOfLabels, 0);
    iter.NextSlice();
  }
}

mitk::Image::Pointer mitk::SliceBasedInterpolationController::Interpolate(unsigned int sliceDimension,
                                                                          unsigned int sliceIndex,
                                                                          const mitk::PlaneGeometry *currentPlane,
                                                                          unsigned int timeStep)
{
  if (m_WorkingImage.IsNull())
    return nullptr;
  if (!currentPlane)
    return nullptr;
  if (timeStep >= m_LabelCountInSlice.size())
    return nullptr;
  if (sliceDimension > 2)
    return nullptr;
  unsigned int upperLimit = m_LabelCountInSlice[timeStep][sliceDimension].size();
  if (sliceIndex >= upperLimit - 1)
    return nullptr; // can't interpolate first and last slice
  if (sliceIndex < 1)
    return nullptr;

  int pixelValue = m_WorkingImage->GetActiveLabel()->GetValue();

  // slice contains a segmentation, won't interpolate anything then
  if (m_LabelCountInSlice[timeStep][sliceDimension][sliceIndex][pixelValue] > 0)
    return nullptr;

  unsigned int lowerBound(0);
  unsigned int upperBound(0);
  bool bounds(false);

  for (lowerBound = sliceIndex - 1; /*lowerBound >= 0*/; --lowerBound)
  {
    if (m_LabelCountInSlice[timeStep][sliceDimension][lowerBound][pixelValue] > 0)
    {
      bounds = true;
      break;
    }

    if (lowerBound == 0)
      break;
  }

  if (!bounds)
    return nullptr;

  bounds = false;
  for (upperBound = sliceIndex + 1; upperBound < upperLimit; ++upperBound)
  {
    if (m_LabelCountInSlice[timeStep][sliceDimension][upperBound][pixelValue] > 0)
    {
      bounds = true;
      break;
    }
  }

  if (!bounds)
    return nullptr;

  // ok, we have found two neighboring slices with the active label
  // (and we made sure that the current slice does NOT contain the active label
  // Setting up the ExtractSliceFilter
  mitk::ExtractSliceFilter::Pointer extractor = ExtractSliceFilter::New();
  extractor->SetInput(m_WorkingImage);
  extractor->SetTimeStep(timeStep);
  extractor->SetResliceTransformByGeometry(m_WorkingImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep));
  extractor->SetVtkOutputRequest(false);

  // Reslicing the current plane
  extractor->SetWorldGeometry(currentPlane);
  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Error in 2D interpolation: " << e.what();
    return nullptr;
  }

  mitk::Image::Pointer resultImage = extractor->GetOutput();
  resultImage->DisconnectPipeline();

  // Creating PlaneGeometry for lower slice
  mitk::PlaneGeometry::Pointer reslicePlane = currentPlane->Clone();

  // Transforming the current origin so that it matches the lower slice
  mitk::Point3D origin = currentPlane->GetOrigin();
  m_WorkingImage->GetSlicedGeometry()->WorldToIndex(origin, origin);
  origin[sliceDimension] = lowerBound;
  m_WorkingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
  reslicePlane->SetOrigin(origin);

  // Extract the lower slice
  extractor->SetWorldGeometry(reslicePlane);
  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Error in 2D interpolation: " << e.what();
    return nullptr;
  }

  mitk::Image::Pointer lowerMITKSlice = extractor->GetOutput();
  lowerMITKSlice->DisconnectPipeline();

  // Transforming the current origin so that it matches the upper slice
  m_WorkingImage->GetSlicedGeometry()->WorldToIndex(origin, origin);
  origin[sliceDimension] = upperBound;
  m_WorkingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
  reslicePlane->SetOrigin(origin);

  // Extract the upper slice
  extractor->SetWorldGeometry(reslicePlane);
  extractor->Modified();

  try
  {
    extractor->Update();
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Error in 2D interpolation: " << e.what();
    return nullptr;
  }

  mitk::Image::Pointer upperMITKSlice = extractor->GetOutput();
  upperMITKSlice->DisconnectPipeline();

  if (lowerMITKSlice.IsNull() || upperMITKSlice.IsNull())
    return nullptr;

  // interpolation algorithm gets some inputs
  //   two segmentations (guaranteed to be of the same data type, but no special data type guaranteed)
  //   orientation (sliceDimension) of the segmentations
  //   position of the two slices (sliceIndices)
  //   one volume image (original patient image)
  //
  // interpolation algorithm can use e.g. itk::ImageSliceConstIteratorWithIndex to
  //   inspect the original patient image at appropriate positions

  mitk::SegmentationInterpolationAlgorithm::Pointer algorithm =
    mitk::ShapeBasedInterpolationAlgorithm::New().GetPointer();

  algorithm->Interpolate(
    lowerMITKSlice.GetPointer(), lowerBound, upperMITKSlice.GetPointer(), upperBound, sliceIndex, 0, resultImage);

  return resultImage;
}
