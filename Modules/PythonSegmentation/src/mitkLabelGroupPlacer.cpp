/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelGroupPlacer.h>

#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

#include <itkMultiThreaderBase.h>

#include <algorithm>
#include <optional>

namespace
{
  using LabelPixelType = mitk::Label::PixelType;

  // Several per thread, so that a thread finishing early takes another rather
  // than waiting for the slowest. Not more, as each of them counts the voxels
  // of every label on its own.
  constexpr std::size_t MAX_NUMBER_OF_CHUNKS = 64;

  std::size_t GetVolumeSize(const mitk::Image* image)
  {
    return static_cast<std::size_t>(image->GetDimension(0)) * image->GetDimension(1) * image->GetDimension(2);
  }

  void CheckMaskSize(const mitk::MaskRuns& mask, const mitk::Image* groupImage)
  {
    if (mask.VolumeSize != GetVolumeSize(groupImage))
      mitkThrow() << "The mask has " << mask.VolumeSize << " voxels, the volumes of the segmentation have " << GetVolumeSize(groupImage) << ".";
  }

  // Calls function(first, last) for consecutive ranges that split [0, count)
  // into chunks, each chunk on a thread of its own, and passes the index of
  // the chunk as well.
  template <typename TFunction>
  void ForEachChunk(std::size_t count, std::size_t numberOfChunks, TFunction function)
  {
    itk::MultiThreaderBase::New()->ParallelizeArray(0, numberOfChunks, [&](itk::SizeValueType chunk)
      {
        function(chunk, count * chunk / numberOfChunks, count * (chunk + 1) / numberOfChunks);
      },
      nullptr);
  }

  std::size_t GetNumberOfChunks(std::size_t count)
  {
    return std::min(count, MAX_NUMBER_OF_CHUNKS);
  }

  // Calls function(voxel) for every voxel of the runs in [first, last).
  template <typename TFunction>
  void ForEachVoxel(const mitk::MaskRuns& mask, std::size_t first, std::size_t last, TFunction function)
  {
    for (auto i = first; i < last; ++i)
    {
      const auto& run = mask.Runs[i];

      for (auto voxel = run.Offset; voxel < run.Offset + run.Length; ++voxel)
        function(voxel);
    }
  }

  void Accumulate(std::vector<std::size_t>& sum, const std::vector<std::size_t>& addend)
  {
    for (std::size_t i = 0; i < sum.size(); ++i)
      sum[i] += addend[i];
  }
}

mitk::MaskRuns mitk::ExtractMaskRuns(const Image* mask)
{
  if (nullptr == mask)
    mitkThrow() << "Cannot extract the voxels of a mask without a mask.";

  if (mask->GetPixelType() != MakeScalarPixelType<unsigned char>())
    mitkThrow() << "Cannot extract the voxels of a mask whose pixel type is not unsigned char.";

  const std::size_t sizeX = mask->GetDimension(0);
  const std::size_t sizeY = mask->GetDimension(1);
  const std::size_t sizeZ = mask->GetDimension(2);

  ImageReadAccessor accessor(mask, mask->GetVolumeData(0));
  const auto* voxels = static_cast<const unsigned char*>(accessor.GetData());

  std::vector<std::vector<MaskRuns::Run>> runsBySlice(sizeZ);

  itk::MultiThreaderBase::New()->ParallelizeArray(0, sizeZ, [&](itk::SizeValueType z)
    {
      auto& runs = runsBySlice[z];

      for (std::size_t y = 0; y < sizeY; ++y)
      {
        const auto rowOffset = (z * sizeY + y) * sizeX;
        const auto* row = voxels + rowOffset;

        for (std::size_t x = 0; x < sizeX;)
        {
          while (x < sizeX && row[x] == 0)
            ++x;

          const auto first = x;

          while (x < sizeX && row[x] != 0)
            ++x;

          if (x > first)
            runs.push_back({ rowOffset + first, x - first });
        }
      }
    },
    nullptr);

  MaskRuns result;
  result.VolumeSize = sizeX * sizeY * sizeZ;

  for (const auto& runs : runsBySlice)
  {
    for (const auto& run : runs)
    {
      result.Runs.push_back(run);
      result.VoxelCount += run.Length;
    }
  }

  return result;
}

mitk::LabelGroupPlacer::LabelGroupPlacer(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation* preview, TimeStepType timeStep)
  : m_Segmentation(segmentation),
    m_Preview(preview),
    m_TimeStep(timeStep)
{
  if (nullptr == segmentation || nullptr == preview)
    mitkThrow() << "Cannot place labels without a segmentation and a preview.";

  if (preview->GetNumberOfGroups() < segmentation->GetNumberOfGroups())
    mitkThrow() << "Cannot place labels into a preview that has fewer groups than the segmentation.";

  for (const auto value : segmentation->GetAllLabelValues())
    m_MaxSegmentationLabelValue = std::max(m_MaxSegmentationLabelValue, value);
}

mitk::LabelGroupPlacer::GroupIndexType mitk::LabelGroupPlacer::FindGroup(const MaskRuns& mask)
{
  const auto segmentationGroups = m_Segmentation->GetNumberOfGroups();
  const auto previewGroups = m_Preview->GetNumberOfGroups();

  std::vector<GroupIndexType> candidates;

  if (segmentationGroups > 0)
    candidates.push_back(m_Segmentation->GetActiveLayer());

  for (GroupIndexType group = 0; group < previewGroups; ++group)
  {
    if (segmentationGroups == 0 || group != m_Segmentation->GetActiveLayer())
      candidates.push_back(group);
  }

  for (const auto group : candidates)
  {
    if (this->Fits(mask, group))
      return group;
  }

  return previewGroups;
}

bool mitk::LabelGroupPlacer::Fits(const MaskRuns& mask, GroupIndexType group)
{
  const auto* previewImage = m_Preview->GetGroupImage(group);
  CheckMaskSize(mask, previewImage);

  std::optional<ImageReadAccessor> segmentationAccessor;
  const LabelPixelType* segmentationPixels = nullptr;

  if (group < m_Segmentation->GetNumberOfGroups())
  {
    const auto* segmentationImage = m_Segmentation->GetGroupImage(group);
    segmentationAccessor.emplace(segmentationImage, segmentationImage->GetVolumeData(m_TimeStep));
    segmentationPixels = static_cast<const LabelPixelType*>(segmentationAccessor->GetData());
  }

  ImageReadAccessor previewAccessor(previewImage, previewImage->GetVolumeData(m_TimeStep));
  const auto* previewPixels = static_cast<const LabelPixelType*>(previewAccessor.GetData());

  const auto maxPreviewLabelValue = m_VoxelCounts.empty() ? LabelValueType(0) : m_VoxelCounts.rbegin()->first;

  // The voxels the mask shares with each label, by label value.
  struct SharedVoxels
  {
    std::vector<std::size_t> Segmentation;
    std::vector<std::size_t> Preview;
  };

  const auto numberOfChunks = GetNumberOfChunks(mask.Runs.size());

  std::vector<SharedVoxels> sharedByChunk(numberOfChunks, SharedVoxels{
    std::vector<std::size_t>(m_MaxSegmentationLabelValue + 1u),
    std::vector<std::size_t>(maxPreviewLabelValue + 1u) });

  ForEachChunk(mask.Runs.size(), numberOfChunks, [&](std::size_t chunk, std::size_t first, std::size_t last)
    {
      auto& shared = sharedByChunk[chunk];

      ForEachVoxel(mask, first, last, [&](std::size_t voxel)
        {
          if (nullptr != segmentationPixels && segmentationPixels[voxel] != 0)
          {
            if (segmentationPixels[voxel] <= m_MaxSegmentationLabelValue)
              ++shared.Segmentation[segmentationPixels[voxel]];

            return;
          }

          if (previewPixels[voxel] != 0 && previewPixels[voxel] <= maxPreviewLabelValue)
            ++shared.Preview[previewPixels[voxel]];
        });
    });

  SharedVoxels shared{
    std::vector<std::size_t>(m_MaxSegmentationLabelValue + 1u),
    std::vector<std::size_t>(maxPreviewLabelValue + 1u) };

  for (const auto& chunkShared : sharedByChunk)
  {
    Accumulate(shared.Segmentation, chunkShared.Segmentation);
    Accumulate(shared.Preview, chunkShared.Preview);
  }

  const auto overlaps = [&mask](std::size_t sharedVoxels, std::size_t otherVoxels)
  {
    return static_cast<double>(sharedVoxels) > OVERLAP_TOLERANCE * static_cast<double>(std::min(mask.VoxelCount, otherVoxels));
  };

  for (std::size_t value = 1; value < shared.Segmentation.size(); ++value)
  {
    if (shared.Segmentation[value] != 0 && overlaps(shared.Segmentation[value], this->GetSegmentationLabelSizes(group)[value]))
      return false;
  }

  for (std::size_t value = 1; value < shared.Preview.size(); ++value)
  {
    if (shared.Preview[value] != 0 && overlaps(shared.Preview[value], this->GetVoxelCount(static_cast<LabelValueType>(value))))
      return false;
  }

  return true;
}

const std::vector<std::size_t>& mitk::LabelGroupPlacer::GetSegmentationLabelSizes(GroupIndexType group)
{
  auto [sizes, inserted] = m_SegmentationLabelSizes.try_emplace(group, m_MaxSegmentationLabelValue + 1u);

  if (!inserted)
    return sizes->second;

  const auto* image = m_Segmentation->GetGroupImage(group);
  ImageReadAccessor accessor(image, image->GetVolumeData(m_TimeStep));
  const auto* pixels = static_cast<const LabelPixelType*>(accessor.GetData());

  const auto volumeSize = GetVolumeSize(image);
  const auto numberOfChunks = GetNumberOfChunks(volumeSize);

  std::vector<std::vector<std::size_t>> sizesByChunk(numberOfChunks, std::vector<std::size_t>(m_MaxSegmentationLabelValue + 1u));

  ForEachChunk(volumeSize, numberOfChunks, [&](std::size_t chunk, std::size_t first, std::size_t last)
    {
      auto& chunkSizes = sizesByChunk[chunk];

      for (auto voxel = first; voxel < last; ++voxel)
      {
        if (pixels[voxel] <= m_MaxSegmentationLabelValue)
          ++chunkSizes[pixels[voxel]];
      }
    });

  for (const auto& chunkSizes : sizesByChunk)
    Accumulate(sizes->second, chunkSizes);

  return sizes->second;
}

void mitk::LabelGroupPlacer::Write(const MaskRuns& mask, GroupIndexType group, LabelValueType value, WriteMode mode)
{
  if (group >= m_Preview->GetNumberOfGroups())
    mitkThrow() << "Cannot write a label into group " << group << ", which the preview does not have.";

  auto* previewImage = m_Preview->GetGroupImage(group);
  CheckMaskSize(mask, previewImage);

  std::optional<ImageReadAccessor> segmentationAccessor;
  const LabelPixelType* segmentationPixels = nullptr;

  if (mode == WriteMode::KeepOccupiedVoxels && group < m_Segmentation->GetNumberOfGroups())
  {
    const auto* segmentationImage = m_Segmentation->GetGroupImage(group);
    segmentationAccessor.emplace(segmentationImage, segmentationImage->GetVolumeData(m_TimeStep));
    segmentationPixels = static_cast<const LabelPixelType*>(segmentationAccessor->GetData());
  }

  const LabelValueType maxPreviewLabelValue = std::max(value, m_VoxelCounts.empty() ? LabelValueType(0) : m_VoxelCounts.rbegin()->first);

  struct WrittenVoxels
  {
    std::size_t Count = 0;

    /** Voxels taken from other labels of the preview, by label value. */
    std::vector<std::size_t> Taken;
  };

  const auto numberOfChunks = GetNumberOfChunks(mask.Runs.size());

  std::vector<WrittenVoxels> writtenByChunk(numberOfChunks, WrittenVoxels{ 0,
    std::vector<std::size_t>(mode == WriteMode::OverwriteVoxels ? maxPreviewLabelValue + 1u : 0u) });

  {
    ImageWriteAccessor previewAccessor(previewImage, previewImage->GetVolumeData(m_TimeStep));
    auto* previewPixels = static_cast<LabelPixelType*>(previewAccessor.GetData());

    ForEachChunk(mask.Runs.size(), numberOfChunks, [&](std::size_t chunk, std::size_t first, std::size_t last)
      {
        auto& written = writtenByChunk[chunk];

        ForEachVoxel(mask, first, last, [&](std::size_t voxel)
          {
            auto& pixel = previewPixels[voxel];

            if (mode == WriteMode::KeepOccupiedVoxels)
            {
              if (pixel != 0 || (nullptr != segmentationPixels && segmentationPixels[voxel] != 0))
                return;
            }
            else
            {
              if (pixel == value)
                return;

              if (pixel != 0 && pixel <= maxPreviewLabelValue)
                ++written.Taken[pixel];
            }

            pixel = value;
            ++written.Count;
          });
      });
  }

  previewImage->Modified();

  for (const auto& written : writtenByChunk)
  {
    m_VoxelCounts[value] += written.Count;

    for (std::size_t other = 1; other < written.Taken.size(); ++other)
    {
      if (written.Taken[other] == 0)
        continue;

      if (auto count = m_VoxelCounts.find(static_cast<LabelValueType>(other)); count != m_VoxelCounts.end())
        count->second -= std::min(count->second, written.Taken[other]);
    }
  }
}

std::size_t mitk::LabelGroupPlacer::GetVoxelCount(LabelValueType value) const
{
  const auto count = m_VoxelCounts.find(value);
  return count != m_VoxelCounts.end() ? count->second : 0;
}
