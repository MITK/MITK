/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkBinaryThresholdTool_h_Included
#define mitkBinaryThresholdTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkAutoSegmentationTool.h"
#include "mitkDataNode.h"

#include <itkImage.h>

class ModuleResource;

namespace mitk
{

  /**
  \brief A tool for image thresholding.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
  */
  class Segmentation_EXPORT BinaryThresholdTool : public AutoSegmentationTool
  {
  public:

    Message3<double,double, bool> IntervalBordersChanged;
    Message1<double>     ThresholdingValueChanged;

    mitkClassMacro(BinaryThresholdTool, AutoSegmentationTool);
    itkNewMacro(BinaryThresholdTool);

    virtual const char** GetXPM() const;
    ModuleResource GetIconResource() const;
    virtual const char* GetName() const;

    virtual void Activated();
    virtual void Deactivated();

    virtual void SetThresholdValue(double value);

    /// \brief Adds the preview image to the current working image.
    virtual void ConfirmSegmentation();

    virtual void CancelThresholding();


  protected:

    BinaryThresholdTool(); // purposely hidden
    virtual ~BinaryThresholdTool();

    void SetupPreviewNodeFor( DataNode* nodeForThresholding );

    void OnRoiDataChanged();

    template <typename TPixel1, unsigned int VDimension1, typename TPixel2, unsigned int VDimension2>
    void ITKThresholding( itk::Image<TPixel1, VDimension1>* targetImage, const itk::Image<TPixel2, VDimension2>* sourceImage );

    DataNode::Pointer m_ThresholdFeedbackNode;
    DataNode::Pointer m_ReferenceNode;
    DataNode::Pointer m_NodeForThresholding;
    int m_CurrentTimeStep;

    double m_SensibleMinimumThresholdValue;
    double m_SensibleMaximumThresholdValue;
    double m_CurrentThresholdValue;
    bool m_IsFloatImage;

  };

} // namespace

#endif

