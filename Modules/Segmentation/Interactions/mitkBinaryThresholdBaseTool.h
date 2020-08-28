/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryThresholdBaseTool_h_Included
#define mitkBinaryThresholdBaseTool_h_Included

#include "mitkAutoSegmentationWithPreviewTool.h"
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
  class MITKSEGMENTATION_EXPORT BinaryThresholdBaseTool : public AutoSegmentationWithPreviewTool
  {
  public:
    Message3<double, double, bool> IntervalBordersChanged;
    Message2<mitk::ScalarType, mitk::ScalarType> ThresholdingValuesChanged;

    mitkClassMacro(BinaryThresholdBaseTool, AutoSegmentationWithPreviewTool);

    virtual void SetThresholdValues(double lower, double upper);

  protected:
    BinaryThresholdBaseTool(); // purposely hidden
    ~BinaryThresholdBaseTool() override;

    itkSetMacro(LockedUpperThreshold, bool);
    itkGetMacro(LockedUpperThreshold, bool);
    itkBooleanMacro(LockedUpperThreshold);

    itkGetMacro(SensibleMinimumThresholdValue, ScalarType);
    itkGetMacro(SensibleMaximumThresholdValue, ScalarType);

    void InitiateToolByInput() override;
    void DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep) override;

  private:
    ScalarType m_SensibleMinimumThresholdValue;
    ScalarType m_SensibleMaximumThresholdValue;
    ScalarType m_CurrentLowerThresholdValue;
    ScalarType m_CurrentUpperThresholdValue;

    /** Indicates if the tool should behave like a single threshold tool (true)
      or like a upper/lower threshold tool (false)*/
    bool m_LockedUpperThreshold = false;
  };

} // namespace

#endif
