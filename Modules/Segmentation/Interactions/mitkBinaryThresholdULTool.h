/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkBinaryThresholdULTool_h_Included
#define mitkBinaryThresholdULTool_h_Included

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkAutoSegmentationTool.h"
#include "mitkDataNode.h"

#include <itkImage.h>
#include <itkBinaryThresholdImageFilter.h>

namespace mitk
{

  /**
  \brief Calculates the segmented volumes for binary images.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
  */
  class MitkExt_EXPORT BinaryThresholdULTool : public AutoSegmentationTool
  {
  public:

    Message2<int,int> IntervalBordersChanged;
    Message2<int, int>     ThresholdingValuesChanged;

    mitkClassMacro(BinaryThresholdULTool, AutoSegmentationTool);
    itkNewMacro(BinaryThresholdULTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

    virtual void Activated();
    virtual void Deactivated();

    virtual void SetThresholdValues(int lower, int upper);
    virtual void AcceptCurrentThresholdValue(const std::string& organName, const Color& color);
    virtual void CancelThresholding();


  protected:

    BinaryThresholdULTool(); // purposely hidden
    virtual ~BinaryThresholdULTool();

    void SetupPreviewNode();

    void CreateNewSegmentationFromThreshold(DataNode* node, const std::string& organType, const Color& color);

    void OnRoiDataChanged();
    void UpdatePreview();

    template <typename TPixel, unsigned int VImageDimension>
    void ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep );

    DataNode::Pointer m_ThresholdFeedbackNode;
    DataNode::Pointer m_OriginalImageNode;
    DataNode::Pointer m_NodeForThresholding;

    int m_SensibleMinimumThresholdValue;
    int m_SensibleMaximumThresholdValue;
    int m_CurrentLowerThresholdValue;
    int m_CurrentUpperThresholdValue;

    typedef itk::Image<int, 3> ImageType;
    typedef itk::Image< Tool::DefaultSegmentationDataType, 3> SegmentationType; // this is sure for new segmentations
    typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;
    ThresholdFilterType::Pointer m_ThresholdFilter;

  };

} // namespace

#endif

