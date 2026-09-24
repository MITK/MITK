/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSegmentationInterpolationController.h>

#include <mitkExceptionMacro.h>
#include <mitkExtractSliceFilter.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkITKImageImport.h>

#include <itkImage.h>
#include <itkMultiThreaderBase.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <mutex>

namespace
{
  using LabelPixelType = mitk::Label::PixelType;
  using SliceCounts = std::array<std::vector<std::size_t>, 3>;
  using CropImageType = itk::Image<LabelPixelType, 2>;

  // Outside both shapes, both signed distances are positive, so the interpolated shape lies within their common
  // bounding box. Inside the box, the distances depend on no pixel farther away than the neighborhood the
  // iso-contour filter reads next to a contour (radius 2), so cropping with this margin leaves the result unchanged.
  constexpr std::size_t CROP_MARGIN = 3;

  SliceCounts CountLabelVoxels(const mitk::Image* image, unsigned int timeStep, LabelPixelType labelValue)
  {
    mitk::ImagePixelReadAccessor<LabelPixelType, 3> accessor(image, image->GetVolumeData(timeStep));
    const auto* voxels = accessor.GetData();

    std::array<std::size_t, 3> size;
    itk::ImageRegion<3>::SizeType regionSize;
    SliceCounts counts;

    for (unsigned int i = 0; i < 3; ++i)
    {
      size[i] = image->GetDimension(i);
      regionSize[i] = size[i];
      counts[i].assign(size[i], 0);
    }

    std::mutex countsMutex;

    itk::MultiThreaderBase::New()->ParallelizeImageRegion<3>(itk::ImageRegion<3>(regionSize), [&](const itk::ImageRegion<3>& chunk)
      {
        SliceCounts chunkCounts;

        for (unsigned int i = 0; i < 3; ++i)
          chunkCounts[i].assign(size[i], 0);

        const auto xBegin = static_cast<std::size_t>(chunk.GetIndex(0));
        const auto xEnd = xBegin + chunk.GetSize(0);
        const auto yBegin = static_cast<std::size_t>(chunk.GetIndex(1));
        const auto yEnd = yBegin + chunk.GetSize(1);
        const auto zBegin = static_cast<std::size_t>(chunk.GetIndex(2));
        const auto zEnd = zBegin + chunk.GetSize(2);

        for (auto z = zBegin; z < zEnd; ++z)
        {
          for (auto y = yBegin; y < yEnd; ++y)
          {
            const auto* row = voxels + (z * size[1] + y) * size[0];

            // Most rows do not contain the label, and std::count is vectorized.
            const auto rowCount = static_cast<std::size_t>(std::count(row + xBegin, row + xEnd, labelValue));

            if (0 == rowCount)
              continue;

            for (auto x = xBegin; x < xEnd; ++x)
              chunkCounts[0][x] += row[x] == labelValue ? 1 : 0;

            chunkCounts[1][y] += rowCount;
            chunkCounts[2][z] += rowCount;
          }
        }

        const std::lock_guard<std::mutex> lock(countsMutex);

        for (unsigned int i = 0; i < 3; ++i)
          std::transform(counts[i].begin(), counts[i].end(), chunkCounts[i].begin(), counts[i].begin(), std::plus<>());
      }, nullptr);

    return counts;
  }

  mitk::PlaneGeometry::Pointer MovePlaneToSlice(const mitk::PlaneGeometry* plane,
                                                const mitk::BaseGeometry* imageGeometry,
                                                unsigned int sliceDimension,
                                                unsigned int sliceIndex)
  {
    auto origin = plane->GetOrigin();
    imageGeometry->WorldToIndex(origin, origin);
    origin[sliceDimension] = sliceIndex;
    imageGeometry->IndexToWorld(origin, origin);

    auto movedPlane = plane->Clone();
    movedPlane->SetOrigin(origin);
    return movedPlane;
  }

  mitk::ExtractSliceFilter::Pointer CreateSliceExtractor(const mitk::Image* image,
                                                         const mitk::PlaneGeometry* plane,
                                                         unsigned int timeStep)
  {
    auto extractor = mitk::ExtractSliceFilter::New();
    extractor->SetInput(image);
    extractor->SetTimeStep(timeStep);
    extractor->SetResliceTransformByGeometry(image->GetTimeGeometry()->GetGeometryForTimeStep(timeStep));
    extractor->SetVtkOutputRequest(false);
    extractor->SetWorldGeometry(plane);
    return extractor;
  }

  mitk::Image::Pointer ExtractSlice(const mitk::Image* image, const mitk::PlaneGeometry* plane, unsigned int timeStep)
  {
    auto extractor = CreateSliceExtractor(image, plane, timeStep);
    extractor->Update();
    return extractor->GetOutput();
  }

  /** The slice the extractor would produce, with all pixels 0 instead of resampled from the image. */
  mitk::Image::Pointer CreateEmptySlice(const mitk::Image* image, const mitk::PlaneGeometry* plane, unsigned int timeStep)
  {
    auto extractor = CreateSliceExtractor(image, plane, timeStep);
    extractor->UpdateOutputInformation();

    // Not the output itself: accessing its data would make the extractor resample the image.
    auto slice = mitk::Image::New();
    slice->Initialize(extractor->GetOutput());

    mitk::ImagePixelWriteAccessor<LabelPixelType, 2> accessor(slice);
    std::fill_n(accessor.GetData(), std::size_t{slice->GetDimension(0)} * slice->GetDimension(1), LabelPixelType{0});
    return slice;
  }

  struct LabelBounds
  {
    std::array<std::size_t, 2> Min = {std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max()};
    std::array<std::size_t, 2> Max = {0, 0};

    bool IsEmpty() const
    {
      return Min[0] > Max[0];
    }
  };

  void ExtendByLabel(LabelBounds& bounds, const mitk::Image* slice, LabelPixelType labelValue)
  {
    mitk::ImagePixelReadAccessor<LabelPixelType, 2> accessor(slice);
    const auto* pixels = accessor.GetData();

    const std::size_t sizeX = slice->GetDimension(0);
    const std::size_t sizeY = slice->GetDimension(1);

    for (std::size_t y = 0; y < sizeY; ++y)
    {
      const auto* rowBegin = pixels + y * sizeX;
      const auto* rowEnd = rowBegin + sizeX;
      const auto* first = std::find(rowBegin, rowEnd, labelValue);

      if (first == rowEnd)
        continue;

      const auto* last = rowEnd - 1;

      while (*last != labelValue)
        --last;

      bounds.Min[0] = std::min(bounds.Min[0], static_cast<std::size_t>(first - rowBegin));
      bounds.Max[0] = std::max(bounds.Max[0], static_cast<std::size_t>(last - rowBegin));
      bounds.Min[1] = std::min(bounds.Min[1], y);
      bounds.Max[1] = std::max(bounds.Max[1], y);
    }
  }

  CropImageType::Pointer CreateCropImage(const std::array<std::size_t, 2>& size, const mitk::Vector3D& spacing)
  {
    CropImageType::SizeType cropSize;
    CropImageType::SpacingType cropSpacing;

    for (unsigned int i = 0; i < 2; ++i)
    {
      cropSize[i] = size[i];
      cropSpacing[i] = spacing[i];
    }

    auto crop = CropImageType::New();
    crop->SetRegions(cropSize);
    crop->SetSpacing(cropSpacing);
    crop->Allocate();
    return crop;
  }

  /** Copies a region of the slice into a new image, with 1 for the label and 0 for everything else. */
  mitk::Image::Pointer CreateBinaryCrop(const mitk::Image* slice,
                                        LabelPixelType labelValue,
                                        const std::array<std::size_t, 2>& cropBegin,
                                        const std::array<std::size_t, 2>& cropSize)
  {
    // The iso-contour filter scales distances by the spacing, so the crop keeps the one of the slice.
    auto crop = CreateCropImage(cropSize, slice->GetGeometry()->GetSpacing());
    auto* cropPixels = crop->GetBufferPointer();

    mitk::ImagePixelReadAccessor<LabelPixelType, 2> accessor(slice);
    const std::size_t sliceSizeX = slice->GetDimension(0);

    for (std::size_t y = 0; y < cropSize[1]; ++y)
    {
      const auto* row = accessor.GetData() + (cropBegin[1] + y) * sliceSizeX + cropBegin[0];

      std::transform(row, row + cropSize[0], cropPixels + y * cropSize[0], [labelValue](LabelPixelType value) {
        return static_cast<LabelPixelType>(value == labelValue ? 1 : 0);
      });
    }

    return mitk::GrabItkImageMemory(crop);
  }

  void PasteCrop(const mitk::Image* crop, const std::array<std::size_t, 2>& cropBegin, mitk::Image* slice)
  {
    mitk::ImagePixelReadAccessor<LabelPixelType, 2> cropAccessor(crop);
    mitk::ImagePixelWriteAccessor<LabelPixelType, 2> sliceAccessor(slice);

    const std::size_t cropSizeX = crop->GetDimension(0);
    const std::size_t cropSizeY = crop->GetDimension(1);
    const std::size_t sliceSizeX = slice->GetDimension(0);

    for (std::size_t y = 0; y < cropSizeY; ++y)
    {
      std::copy_n(cropAccessor.GetData() + y * cropSizeX,
                  cropSizeX,
                  sliceAccessor.GetData() + (cropBegin[1] + y) * sliceSizeX + cropBegin[0]);
    }
  }
}

mitk::SegmentationInterpolationController::SegmentationInterpolationController()
  : m_LabelValue(Label::UNLABELED_VALUE)
{
}

mitk::SegmentationInterpolationController::~SegmentationInterpolationController() = default;

void mitk::SegmentationInterpolationController::SetSegmentationVolume(const Image *segmentation, Label::PixelType labelValue)
{
  if (m_Segmentation.GetPointer() == segmentation && m_LabelValue == labelValue)
    return;

  if (nullptr != segmentation)
  {
    if (segmentation->GetDimension() < 3 || segmentation->GetDimension() > 4)
      mitkThrow() << "2D interpolation needs a 3D or 3D+t segmentation.";

    if (segmentation->GetPixelType() != MakeScalarPixelType<Label::PixelType>())
      mitkThrow() << "2D interpolation needs a segmentation with label values as pixels.";
  }

  m_Segmentation = segmentation;
  m_LabelValue = labelValue;
  m_SliceCounts.assign(nullptr != segmentation ? segmentation->GetTimeSteps() : 0, {});
  m_EnclosingSlices.reset();

  this->Modified();
}

const mitk::SegmentationInterpolationController::SliceCountsType &mitk::SegmentationInterpolationController::GetSliceCounts(
  unsigned int timeStep)
{
  auto &sliceCounts = m_SliceCounts[timeStep];
  const auto segmentationMTime = m_Segmentation->GetMTime();

  if (sliceCounts.SegmentationMTime != segmentationMTime)
  {
    sliceCounts.Counts = CountLabelVoxels(m_Segmentation, timeStep, m_LabelValue);
    sliceCounts.SegmentationMTime = segmentationMTime;
  }

  return sliceCounts.Counts;
}

mitk::SegmentationInterpolationController::EnclosingSlices mitk::SegmentationInterpolationController::CreateEnclosingSlices(
  unsigned int sliceDimension,
  unsigned int lowerIndex,
  unsigned int upperIndex,
  const PlaneGeometry *plane,
  unsigned int timeStep) const
{
  const auto *imageGeometry = m_Segmentation->GetGeometry(timeStep);
  const auto lowerPlane = MovePlaneToSlice(plane, imageGeometry, sliceDimension, lowerIndex);
  const auto upperPlane = MovePlaneToSlice(plane, imageGeometry, sliceDimension, upperIndex);
  const auto lowerSlice = ExtractSlice(m_Segmentation, lowerPlane, timeStep);
  const auto upperSlice = ExtractSlice(m_Segmentation, upperPlane, timeStep);

  const std::array<std::size_t, 2> sliceSize = {lowerSlice->GetDimension(0), lowerSlice->GetDimension(1)};

  if (upperSlice->GetDimension(0) != sliceSize[0] || upperSlice->GetDimension(1) != sliceSize[1])
  {
    mitkThrowException(SegmentationInterpolationException)
      << "The regions of the slices for the 2D interpolation are not equally sized.";
  }

  EnclosingSlices enclosingSlices;
  enclosingSlices.TimeStep = timeStep;
  enclosingSlices.SliceDimension = sliceDimension;
  enclosingSlices.LowerIndex = lowerIndex;
  enclosingSlices.UpperIndex = upperIndex;
  enclosingSlices.SegmentationMTime = m_Segmentation->GetMTime();
  enclosingSlices.LowerPlane = lowerPlane;
  enclosingSlices.SliceSize = sliceSize;
  enclosingSlices.Algorithm = ShapeBasedInterpolationAlgorithm::New();

  LabelBounds bounds;
  ExtendByLabel(bounds, lowerSlice, m_LabelValue);
  ExtendByLabel(bounds, upperSlice, m_LabelValue);

  if (!bounds.IsEmpty())
  {
    for (unsigned int i = 0; i < 2; ++i)
    {
      const auto cropEnd = std::min(bounds.Max[i] + CROP_MARGIN + 1, sliceSize[i]);
      enclosingSlices.CropBegin[i] = bounds.Min[i] - std::min(bounds.Min[i], CROP_MARGIN);
      enclosingSlices.CropSize[i] = cropEnd - enclosingSlices.CropBegin[i];
    }

    enclosingSlices.LowerCrop =
      CreateBinaryCrop(lowerSlice, m_LabelValue, enclosingSlices.CropBegin, enclosingSlices.CropSize);
    enclosingSlices.UpperCrop =
      CreateBinaryCrop(upperSlice, m_LabelValue, enclosingSlices.CropBegin, enclosingSlices.CropSize);
  }

  return enclosingSlices;
}

const mitk::SegmentationInterpolationController::EnclosingSlices &mitk::SegmentationInterpolationController::GetEnclosingSlices(
  unsigned int sliceDimension,
  unsigned int lowerIndex,
  unsigned int upperIndex,
  const PlaneGeometry *currentPlane,
  unsigned int timeStep)
{
  const auto lowerPlane = MovePlaneToSlice(currentPlane, m_Segmentation->GetGeometry(timeStep), sliceDimension, lowerIndex);

  if (m_EnclosingSlices.has_value() &&
      m_EnclosingSlices->TimeStep == timeStep &&
      m_EnclosingSlices->SliceDimension == sliceDimension &&
      m_EnclosingSlices->LowerIndex == lowerIndex &&
      m_EnclosingSlices->UpperIndex == upperIndex &&
      m_EnclosingSlices->SegmentationMTime == m_Segmentation->GetMTime() &&
      mitk::Equal(*m_EnclosingSlices->LowerPlane, *lowerPlane, mitk::eps))
  {
    return *m_EnclosingSlices;
  }

  m_EnclosingSlices.reset();
  m_EnclosingSlices = this->CreateEnclosingSlices(sliceDimension, lowerIndex, upperIndex, currentPlane, timeStep);
  return *m_EnclosingSlices;
}

mitk::Image::Pointer mitk::SegmentationInterpolationController::InterpolateBetween(const EnclosingSlices &enclosingSlices,
                                                                                   unsigned int sliceIndex,
                                                                                   const PlaneGeometry *slicePlane,
                                                                                   unsigned int timeStep) const
{
  if (enclosingSlices.LowerCrop.IsNull())
    return nullptr;

  auto result = CreateEmptySlice(m_Segmentation, slicePlane, timeStep);

  if (result->GetDimension(0) != enclosingSlices.SliceSize[0] || result->GetDimension(1) != enclosingSlices.SliceSize[1])
  {
    mitkThrowException(SegmentationInterpolationException)
      << "The regions of the slices for the 2D interpolation are not equally sized.";
  }

  auto resultCrop = CreateCropImage(enclosingSlices.CropSize, result->GetGeometry()->GetSpacing());
  auto resultCropImage = GrabItkImageMemory(resultCrop);

  enclosingSlices.Algorithm->Interpolate(enclosingSlices.LowerCrop,
                                         enclosingSlices.LowerIndex,
                                         enclosingSlices.UpperCrop,
                                         enclosingSlices.UpperIndex,
                                         sliceIndex,
                                         enclosingSlices.SliceDimension,
                                         resultCropImage,
                                         timeStep,
                                         nullptr);

  PasteCrop(resultCropImage, enclosingSlices.CropBegin, result);
  return result;
}

mitk::Image::Pointer mitk::SegmentationInterpolationController::Interpolate(unsigned int sliceDimension,
                                                                            unsigned int sliceIndex,
                                                                            const PlaneGeometry *currentPlane,
                                                                            unsigned int timeStep)
{
  if (m_Segmentation.IsNull() || nullptr == currentPlane)
    return nullptr;

  if (timeStep >= m_SliceCounts.size() || sliceDimension > 2)
    return nullptr;

  const auto &counts = this->GetSliceCounts(timeStep)[sliceDimension];

  if (sliceIndex >= counts.size() || counts[sliceIndex] > 0)
    return nullptr;

  const auto isSegmented = [](std::size_t count) { return count > 0; };
  const auto slice = counts.begin() + sliceIndex;
  const auto lower = std::find_if(std::make_reverse_iterator(slice), counts.rend(), isSegmented);
  const auto upper = std::find_if(slice + 1, counts.end(), isSegmented);

  if (lower == counts.rend() || upper == counts.end())
    return nullptr;

  const auto lowerIndex = static_cast<unsigned int>(lower.base() - counts.begin() - 1);
  const auto upperIndex = static_cast<unsigned int>(upper - counts.begin());

  const auto &enclosingSlices = this->GetEnclosingSlices(sliceDimension, lowerIndex, upperIndex, currentPlane, timeStep);
  return this->InterpolateBetween(enclosingSlices, sliceIndex, currentPlane, timeStep);
}

void mitk::SegmentationInterpolationController::InterpolateAll(
  unsigned int sliceDimension,
  const PlaneGeometry *plane,
  unsigned int timeStep,
  const std::function<void(unsigned int sliceIndex, const Image *interpolation)> &consumer)
{
  if (m_Segmentation.IsNull() || nullptr == plane)
    return;

  if (timeStep >= m_SliceCounts.size() || sliceDimension > 2)
    return;

  // Copied, so that the gaps stay those of the segmentation as it was when the call started.
  const auto counts = this->GetSliceCounts(timeStep)[sliceDimension];
  const auto *imageGeometry = m_Segmentation->GetGeometry(timeStep);

  std::optional<unsigned int> lowerIndex;

  for (unsigned int upperIndex = 0; upperIndex < counts.size(); ++upperIndex)
  {
    if (0 == counts[upperIndex])
      continue;

    if (lowerIndex.has_value() && upperIndex - *lowerIndex > 1)
    {
      const auto enclosingSlices = this->CreateEnclosingSlices(sliceDimension, *lowerIndex, upperIndex, plane, timeStep);

      for (auto sliceIndex = *lowerIndex + 1; sliceIndex < upperIndex; ++sliceIndex)
      {
        const auto slicePlane = MovePlaneToSlice(plane, imageGeometry, sliceDimension, sliceIndex);
        const auto interpolation = this->InterpolateBetween(enclosingSlices, sliceIndex, slicePlane, timeStep);

        if (interpolation.IsNotNull())
          consumer(sliceIndex, interpolation);
      }
    }

    lowerIndex = upperIndex;
  }
}
