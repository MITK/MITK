/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryThresholdBaseTool_h_Included
#define mitkBinaryThresholdBaseTool_h_Included

#include "mitkAutoSegmentationTool.h"
#include "mitkCommon.h"
#include "mitkDataNode.h"
#include <MitkSegmentationExports.h>

#include <itkBinaryThresholdImageFilter.h>
#include <itkImage.h>

namespace mitk
{
  /**
  \brief Base class for binary threshold tools.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation
  */
  class MITKSEGMENTATION_EXPORT BinaryThresholdBaseTool : public AutoSegmentationTool
  {
  public:
    Message3<double, double, bool> IntervalBordersChanged;
    Message2<mitk::ScalarType, mitk::ScalarType> ThresholdingValuesChanged;

    mitkClassMacro(BinaryThresholdBaseTool, AutoSegmentationTool);

    void Activated() override;
    void Deactivated() override;

    virtual void SetThresholdValues(double lower, double upper);

    virtual void AcceptCurrentThresholdValue();
    virtual void CancelThresholding();

    itkSetMacro(CreateAllTimeSteps, bool);
    itkGetMacro(CreateAllTimeSteps, bool);
    itkBooleanMacro(CreateAllTimeSteps);

  protected:
    BinaryThresholdBaseTool(); // purposely hidden
    ~BinaryThresholdBaseTool() override;

    itkSetMacro(LockedUpperThreshold, bool);
    itkGetMacro(LockedUpperThreshold, bool);
    itkBooleanMacro(LockedUpperThreshold);

    itkGetMacro(SensibleMinimumThresholdValue, ScalarType);
    itkGetMacro(SensibleMaximumThresholdValue, ScalarType);

  private:
    void SetupPreviewNode();

    void TransferImageAtTimeStep(const Image* sourceImage, Image* destinationImage, const TimeStepType timeStep);

    void CreateNewSegmentationFromThreshold();

    void OnRoiDataChanged();
    void OnTimePointChanged();

    void UpdatePreview();

    DataNode::Pointer m_ThresholdFeedbackNode;
    DataNode::Pointer m_OriginalImageNode;
    DataNode::Pointer m_NodeForThresholding;

    ScalarType m_SensibleMinimumThresholdValue;
    ScalarType m_SensibleMaximumThresholdValue;
    ScalarType m_CurrentLowerThresholdValue;
    ScalarType m_CurrentUpperThresholdValue;

    bool m_IsOldBinary = false;

    /** Indicates if Accepting the threshold should transfer/create the segmentations
     of all time steps (true) or only of the currently selected timepoint (false).*/
    bool m_CreateAllTimeSteps = false;

    /** Indicates if the tool should behave like a single threshold tool (true)
      or like a upper/lower threshold tool (false)*/
    bool m_LockedUpperThreshold = false;
  };

} // namespace

#endif
