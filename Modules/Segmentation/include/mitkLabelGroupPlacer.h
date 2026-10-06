/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelGroupPlacer_h
#define mitkLabelGroupPlacer_h

#include <mitkLabelSetImage.h>
#include <MitkSegmentationExports.h>

#include <map>
#include <vector>

namespace mitk
{
  /** \brief The voxels of a binary mask, as runs of consecutive voxels in memory order. */
  struct MITKSEGMENTATION_EXPORT MaskRuns
  {
    struct Run
    {
      std::size_t Offset; /**< \brief Index of the first voxel in the volume. */
      std::size_t Length; /**< \brief Number of voxels. */
    };

    /** \brief In ascending order of their offsets, without overlaps. */
    std::vector<Run> Runs;

    std::size_t VoxelCount = 0;

    /** \brief Number of voxels of the volume the mask was extracted from. */
    std::size_t VolumeSize = 0;
  };

  /** \brief Returns the voxels of a 3D mask of pixel type unsigned char that are not zero.
   *
   * \throw mitk::Exception if the mask is not such an image.
   */
  MITKSEGMENTATION_EXPORT MaskRuns ExtractMaskRuns(const Image* mask);

  /** \brief Writes masks as labels into the groups of a preview, so that a
   *         label overlaps neither the labels of a segmentation nor the other
   *         labels of the preview.
   *
   * Group i of the preview stands for group i of the segmentation, and groups
   * of the preview beyond those of the segmentation stand for groups that are
   * to be added to it. FindGroup() tells where a mask fits, Write() puts it
   * there. Both work on one time step.
   *
   * Masks of neighboring structures touch, and as the masks are computed
   * independently of each other, they share a few voxels at the border. That
   * is not an overlap as long as it stays within the overlap tolerance.
   */
  class MITKSEGMENTATION_EXPORT LabelGroupPlacer
  {
  public:
    using GroupIndexType = MultiLabelSegmentation::GroupIndexType;
    using LabelValueType = MultiLabelSegmentation::LabelValueType;

    enum class WriteMode
    {
      KeepOccupiedVoxels, /**< \brief Voxels of other labels of the group stay theirs. */
      OverwriteVoxels     /**< \brief The mask takes all its voxels, even from other labels of the preview. */
    };

    /**
     * \param segmentation The labels that the masks must not overlap.
     * \param preview Receives the masks. Has at least the groups of the segmentation, and no
     *        pixels at the time step but those that Write() puts there.
     * \param timeStep The time step of both that the masks are for.
     * \param overlapTolerance Share of the voxels of the smaller of two labels that they may
     *        have in common without overlapping, between 0 and 1. It depends on how precisely
     *        the masks follow the borders of the structures.
     *
     * \throw mitk::Exception if the segmentation or the preview is missing, the preview has
     *        fewer groups than the segmentation, or the tolerance is out of range.
     */
    LabelGroupPlacer(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation* preview, TimeStepType timeStep, double overlapTolerance);

    /** \brief Returns the first group of the preview in which the mask overlaps no label.
     *
     * Tries the active group of the segmentation first, then its other groups
     * in order, then the groups of the preview beyond those. Returns the
     * number of groups of the preview if the mask fits into none of them, so
     * a group has to be added.
     *
     * \pre The mask is of the size of the volumes of the preview.
     */
    GroupIndexType FindGroup(const MaskRuns& mask);

    /** \brief Writes the mask as a label into a group of the preview.
     *
     * \pre The group exists in the preview, and the mask is of the size of its volumes.
     */
    void Write(const MaskRuns& mask, GroupIndexType group, LabelValueType value, WriteMode mode);

    /** \brief Returns how many voxels of the label Write() left in the preview. */
    std::size_t GetVoxelCount(LabelValueType value) const;

  private:
    /** \brief Returns the voxel counts of the labels of a group of the segmentation, by label value. */
    const std::vector<std::size_t>& GetSegmentationLabelSizes(GroupIndexType group);

    bool Fits(const MaskRuns& mask, GroupIndexType group);

    const MultiLabelSegmentation* m_Segmentation;
    MultiLabelSegmentation* m_Preview;
    TimeStepType m_TimeStep;
    double m_OverlapTolerance;

    /** \brief Pixels above it belong to no label of the segmentation and do not count as an overlap. */
    LabelValueType m_MaxSegmentationLabelValue = 0;

    std::map<GroupIndexType, std::vector<std::size_t>> m_SegmentationLabelSizes;
    std::map<LabelValueType, std::size_t> m_VoxelCounts;
  };
}

#endif
