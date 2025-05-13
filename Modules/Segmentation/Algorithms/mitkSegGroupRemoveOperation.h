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

  /** \brief An Operation for applying an edited slice to the a group of a MultiLabelSegmentation.
    \sa SegGroupOperationApplier
    This Operation can be used to realize undo-redo functionality for e.g. segmentation purposes.
  */
  class MITKSEGMENTATION_EXPORT SegGroupRemoveOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegGroupRemoveOperation, SegChangeOperationBase);

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief */
    SegGroupRemoveOperation(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& removalGroupIDs);

    ~SegGroupRemoveOperation() override = default;

    GroupIndexSetType GetGroupIDs() const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegGroupRemoveOperation(const SegGroupRemoveOperation&) = delete;
    SegGroupRemoveOperation& operator=(const SegGroupRemoveOperation&) = delete;

  protected:
    GroupIndexSetType m_GroupIDs;
  };

}
#endif
