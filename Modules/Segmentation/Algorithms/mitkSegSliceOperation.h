/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegSliceOperation_h
#define mitkSegSliceOperation_h

#include "mitkCompressedImageContainer.h"
#include <MitkSegmentationExports.h>
#include <mitkSegChangeOperationBase.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for applying an edited slice to the a group of a MultiLabelSegmentation.
    \sa SegSliceOperationApplier
    This Operation can be used to realize undo-redo functionality for e.g. segmentation purposes.
  */
  class MITKSEGMENTATION_EXPORT SegSliceOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegSliceOperation, SegChangeOperationBase);

    /** \brief */
    SegSliceOperation(MultiLabelSegmentation* segmentation,
                       MultiLabelSegmentation::GroupIndexType groupID,
                       const Image *slice,
                       const TimeStepType timestep,
                       const PlaneGeometry * planeGeometry);
    ~SegSliceOperation() override = default;

    /** \brief Check if it is a valid operation.*/
    bool IsValid() const override;

    /** \brief Get the slice that is applied in the operation.*/
    Image::Pointer GetSlice() const;
    /** \brief Get the time step the operation should be applied on.*/
    TimeStepType GetTimeStep() const;
    /** \brief Get the plane where the slice has to be applied in the volume.*/
    const PlaneGeometry* GetSlicePlaneGeometry() const;
    /** \brief Get the group index of the group image that should be modified.*/
    MultiLabelSegmentation::GroupIndexType GetGroupID() const;

  protected:
    MultiLabelSegmentation::GroupIndexType m_GroupID;
    TimeStepType m_TimeStep;
    CompressedImageContainer m_CompressedImageContainer;
    PlaneGeometry::ConstPointer m_PlaneGeometry;
    /** Ensures that the reference geometry of the plane geometry is not deleted to soon
     see bug T12338.*/
    mitk::BaseGeometry::ConstPointer m_GuardReferenceGeometry;
  };
}
#endif
