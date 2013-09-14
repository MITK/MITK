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

#ifndef mitkBinaryThresholdULTool_h_Included
#define mitkBinaryThresholdULTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkAutoSegmentationTool.h"
#include "mitkDataNode.h"

#include <itkImage.h>
#include <itkBinaryThresholdImageFilter.h>

namespace us {
class ModuleResource;
}

namespace mitk
{

  /**
  \brief Calculates the segmented volumes for binary images.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
  */
  class Segmentation_EXPORT BinaryThresholdULTool : public AutoSegmentationTool
  {
  public:

    Message2<mitk::ScalarType,mitk::ScalarType> IntervalBordersChanged;
    Message2<mitk::ScalarType, mitk::ScalarType>     ThresholdingValuesChanged;

    mitkClassMacro(BinaryThresholdULTool, AutoSegmentationTool);
    itkNewMacro(BinaryThresholdULTool);

    virtual const char** GetXPM() const;
    us::ModuleResource GetIconResource() const;
    virtual const char* GetName() const;

    virtual void Activated();
    virtual void Deactivated();

    virtual void SetThresholdValues(int lower, int upper);
    virtual void AcceptCurrentThresholdValue();
    virtual void CancelThresholding();


  protected:

    BinaryThresholdULTool(); // purposely hidden
    virtual ~BinaryThresholdULTool();

    void SetupPreviewNode();

    void CreateNewSegmentationFromThreshold(DataNode* node);

    void OnRoiDataChanged();
    void UpdatePreview();

    template <typename TPixel, unsigned int VImageDimension>
    void ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep );

    DataNode::Pointer m_ThresholdFeedbackNode;
    DataNode::Pointer m_OriginalImageNode;
    DataNode::Pointer m_NodeForThresholding;

    mitk::ScalarType m_SensibleMinimumThresholdValue;
    mitk::ScalarType m_SensibleMaximumThresholdValue;
    mitk::ScalarType m_CurrentLowerThresholdValue;
    mitk::ScalarType m_CurrentUpperThresholdValue;

    typedef itk::Image<int, 3> ImageType;
    typedef itk::Image< Tool::DefaultSegmentationDataType, 3> SegmentationType; // this is sure for new segmentations
    typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;
    ThresholdFilterType::Pointer m_ThresholdFilter;

  };

} // namespace

#endif

