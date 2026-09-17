/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAndMaskGenerator.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkITKImageImport.h>
#include <mitkNodePredicateGeometry.h>

#include <itkCastImageFilter.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace
{
  using LabelValueType = mitk::AndMaskGenerator::LabelValueType;
  using IndexArrayType = std::array<std::int64_t, 3>;

  template <typename TPixel, unsigned int VImageDimension>
  void CastToLabelImage(const itk::Image<TPixel, VImageDimension>* image, mitk::Image::Pointer& result)
  {
    using InputImageType = itk::Image<TPixel, VImageDimension>;
    using LabelImageType = itk::Image<LabelValueType, VImageDimension>;

    auto castFilter = itk::CastImageFilter<InputImageType, LabelImageType>::New();
    castFilter->SetInput(image);
    castFilter->Update();

    result = mitk::GrabItkImageMemory(castFilter->GetOutput(), nullptr, nullptr, false);
  }

  /** The masks are combined on raw LabelValueType buffers. Masks of another pixel type are
   *  converted into a copy that keeps the geometry of the original mask. */
  mitk::Image::ConstPointer EnsureLabelPixelType(const mitk::Image* mask, const std::string& role)
  {
    if (mask->GetDimension() > 3)
    {
      mitkThrow() << "Cannot combine masks. The " << role << " mask has to be a 2D or 3D image, but has dimension "
                  << mask->GetDimension() << ".";
    }

    if (mask->GetPixelType() == mitk::MakeScalarPixelType<LabelValueType>())
    {
      return mask;
    }

    mitk::Image::Pointer converted;
    AccessByItk_1(mask, CastToLabelImage, converted);
    converted->SetClonedGeometry(mask->GetGeometry());

    return converted;
  }

  /** Integer mapping from primary to secondary voxel indices; exists because both grids are aligned. */
  struct GridMapping
  {
    IndexArrayType base;                // secondary index of the primary index (0, 0, 0)
    std::array<IndexArrayType, 3> step; // change of the secondary index per primary index step along each primary axis
  };

  GridMapping ComputeGridMapping(const mitk::BaseGeometry* primary,
                                 const mitk::BaseGeometry* secondary,
                                 const IndexArrayType& primaryDimensions)
  {
    if (primary->GetImageGeometry() != secondary->GetImageGeometry())
    {
      mitkThrow() << "Cannot combine masks. Primary and secondary mask disagree on whether their geometries are image geometries.";
    }

    // Deviations from the grid are judged in mm so that the criterion does not depend on the
    // voxel size and agrees with the precision the geometry node predicates accept.
    const auto& secondarySpacing = secondary->GetSpacing();
    const auto toGridIndex = [&secondarySpacing](double value, unsigned int axis)
    {
      const double rounded = std::round(value);

      if (std::abs(value - rounded) * secondarySpacing[axis] > mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION)
      {
        mitkThrow() << "Cannot combine masks. The voxel grids of primary and secondary mask are not aligned "
                    << "(deviation of " << value - rounded << " voxels along secondary axis " << axis
                    << "). Both masks need the same orientation, compatible spacings and no sub-voxel offset.";
      }

      return static_cast<std::int64_t>(rounded);
    };

    GridMapping mapping{};

    mitk::Point3D primaryOrigin;
    primaryOrigin.Fill(0.0);
    mitk::Point3D world;
    mitk::Point3D secondaryIndex;
    primary->IndexToWorld(primaryOrigin, world);
    secondary->WorldToIndex(world, secondaryIndex);

    for (unsigned int component = 0; component < 3; ++component)
    {
      mapping.base[component] = toGridIndex(secondaryIndex[component], component);
    }

    for (unsigned int axis = 0; axis < 3; ++axis)
    {
      // An axis with extent 1 (the normal of a 2D mask) is never stepped along, so its
      // spacing does not have to match the secondary grid.
      if (primaryDimensions[axis] <= 1)
      {
        continue;
      }

      mitk::Vector3D primaryStep;
      primaryStep.Fill(0.0);
      primaryStep[axis] = 1.0;
      mitk::Vector3D worldStep;
      mitk::Vector3D secondaryStep;
      primary->IndexToWorld(primaryStep, worldStep);
      secondary->WorldToIndex(worldStep, secondaryStep);

      for (unsigned int component = 0; component < 3; ++component)
      {
        mapping.step[axis][component] = toGridIndex(secondaryStep[component], component);
      }
    }

    return mapping;
  }

  IndexArrayType GetDimensions(const mitk::Image* image)
  {
    return { image->GetDimension(0), image->GetDimension(1), image->GetDimension(2) };
  }

  bool IsSelected(const IndexArrayType& index,
                  const IndexArrayType& dimensions,
                  const LabelValueType* data,
                  LabelValueType selectedLabel)
  {
    for (unsigned int component = 0; component < 3; ++component)
    {
      if (index[component] < 0 || index[component] >= dimensions[component])
      {
        return false;
      }
    }

    return data[index[0] + dimensions[0] * (index[1] + dimensions[1] * index[2])] == selectedLabel;
  }

  mitk::Image::Pointer RestrictToSelectedLabel(const mitk::Image* primary,
                                               const mitk::Image* secondary,
                                               LabelValueType selectedLabel)
  {
    const auto primaryDimensions = GetDimensions(primary);
    const auto secondaryDimensions = GetDimensions(secondary);
    const auto mapping = ComputeGridMapping(primary->GetGeometry(), secondary->GetGeometry(), primaryDimensions);

    mitk::Image::Pointer result = primary->Clone();

    {
      mitk::ImageReadAccessor secondaryAccess(secondary);
      mitk::ImageWriteAccessor resultAccess(result);
      const auto* secondaryData = static_cast<const LabelValueType*>(secondaryAccess.GetData());
      auto* resultData = static_cast<LabelValueType*>(resultAccess.GetData());

      std::size_t offset = 0;

      // The secondary index is stepped along instead of being recomputed per voxel, which is
      // exact because the steps are integers.
      for (std::int64_t k = 0; k < primaryDimensions[2]; ++k)
      {
        IndexArrayType sliceIndex;

        for (unsigned int component = 0; component < 3; ++component)
        {
          sliceIndex[component] = mapping.base[component] + k * mapping.step[2][component];
        }

        for (std::int64_t j = 0; j < primaryDimensions[1]; ++j)
        {
          IndexArrayType secondaryIndex;

          for (unsigned int component = 0; component < 3; ++component)
          {
            secondaryIndex[component] = sliceIndex[component] + j * mapping.step[1][component];
          }

          for (std::int64_t i = 0; i < primaryDimensions[0]; ++i, ++offset)
          {
            if (resultData[offset] != 0 &&
                !IsSelected(secondaryIndex, secondaryDimensions, secondaryData, selectedLabel))
            {
              resultData[offset] = 0;
            }

            for (unsigned int component = 0; component < 3; ++component)
            {
              secondaryIndex[component] += mapping.step[0][component];
            }
          }
        }
      }
    }

    result->Modified();

    return result;
  }
}

void mitk::AndMaskGenerator::SetSecondaryLabelValue(LabelValueType labelValue)
{
  if (m_SecondaryLabelValue != labelValue)
  {
    m_SecondaryLabelValue = labelValue;
    this->Modified();
  }
}

std::optional<mitk::AndMaskGenerator::LabelValueType> mitk::AndMaskGenerator::GetSecondaryLabelValue() const
{
  return m_SecondaryLabelValue;
}

void mitk::AndMaskGenerator::SetInputImage(const Image*)
{
  mitkThrow() << "Invalid call. AndMaskGenerator has no input image of its own. "
              << "Set the input image of the chained mask generators instead.";
}

unsigned int mitk::AndMaskGenerator::GetNumberOfMasks() const
{
  if (m_PrimaryMaskGenerator.IsNull())
  {
    mitkThrow() << "Invalid state. Cannot get number of masks. Primary mask generator is not set.";
  }

  return m_PrimaryMaskGenerator->GetNumberOfMasks();
}

mitk::Image::ConstPointer mitk::AndMaskGenerator::GetReferenceImage()
{
  if (m_PrimaryMaskGenerator.IsNull())
  {
    mitkThrow() << "Invalid state. Cannot get reference image. Primary mask generator is not set.";
  }

  // The primary generator may derive its reference image from the mask it would generate, so it
  // needs the time point here just as much as in DoGetMask().
  m_PrimaryMaskGenerator->SetTimePoint(m_TimePoint);

  return m_PrimaryMaskGenerator->GetReferenceImage();
}

itk::ModifiedTimeType mitk::AndMaskGenerator::GetMTime() const
{
  auto mTime = Superclass::GetMTime();

  if (m_PrimaryMaskGenerator.IsNotNull())
  {
    mTime = std::max(mTime, m_PrimaryMaskGenerator->GetMTime());
  }

  if (m_SecondaryMaskGenerator.IsNotNull())
  {
    mTime = std::max(mTime, m_SecondaryMaskGenerator->GetMTime());
  }

  return mTime;
}

mitk::Image::ConstPointer mitk::AndMaskGenerator::DoGetMask(unsigned int maskID)
{
  if (m_PrimaryMaskGenerator.IsNull())
  {
    mitkThrow() << "Invalid state. Cannot generate mask. Primary mask generator is not set.";
  }

  if (m_SecondaryMaskGenerator.IsNull())
  {
    mitkThrow() << "Invalid state. Cannot generate mask. Secondary mask generator is not set.";
  }

  if (!m_SecondaryLabelValue.has_value())
  {
    mitkThrow() << "Invalid state. Cannot generate mask. Secondary label value is not set.";
  }

  m_PrimaryMaskGenerator->SetTimePoint(m_TimePoint);
  m_SecondaryMaskGenerator->SetTimePoint(m_TimePoint);

  auto primaryMask = m_PrimaryMaskGenerator->GetMask(maskID);
  auto secondaryMask = m_SecondaryMaskGenerator->GetMask(m_SecondaryMaskID);

  if (primaryMask.IsNull())
  {
    mitkThrow() << "Cannot generate mask. Primary mask generator returned no mask for mask ID " << maskID << ".";
  }

  if (secondaryMask.IsNull())
  {
    mitkThrow() << "Cannot generate mask. Secondary mask generator returned no mask for mask ID "
                << m_SecondaryMaskID << ".";
  }

  primaryMask = EnsureLabelPixelType(primaryMask, "primary");
  secondaryMask = EnsureLabelPixelType(secondaryMask, "secondary");

  return RestrictToSelectedLabel(primaryMask, secondaryMask, *m_SecondaryLabelValue);
}
