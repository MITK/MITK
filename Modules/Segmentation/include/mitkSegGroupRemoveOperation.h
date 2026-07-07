/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegGroupRemoveOperation_h
#define mitkSegGroupRemoveOperation_h

#include <mitkSegChangeOperationBase.h>

#include <MitkSegmentationExports.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for removing groups from a MultiLabelSegmentation.

    \sa SegChangeOperationApplier

    This Operation can be used to realize undo-redo functionality for group
    removal in segmentation workflows. It stores the indices of the groups to be removed.
  */
  class MITKSEGMENTATION_EXPORT SegGroupRemoveOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegGroupRemoveOperation, SegChangeOperationBase);

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief Construct a remove operation for the specified groups.
      \param segmentation The target segmentation.
      \param removalGroupIDs The set of group indices to remove.
    */
    SegGroupRemoveOperation(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& removalGroupIDs);

    ~SegGroupRemoveOperation() override = default;

    /** \brief Get the group indices to be removed. */
    GroupIndexSetType GetGroupIDs() const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegGroupRemoveOperation(const SegGroupRemoveOperation&) = delete;
    SegGroupRemoveOperation& operator=(const SegGroupRemoveOperation&) = delete;

  protected:
    GroupIndexSetType m_GroupIDs;
  };

}
#endif
