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

#ifndef mitkBinaryThresholdTool_h_Included
#define mitkBinaryThresholdTool_h_Included

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkAutoSegmentationTool.h"
#include "mitkDataNode.h"

#include <itkImage.h>

namespace mitk
{

  /**
  \brief Calculates the segmented volumes for binary images.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
  */
  class MitkExt_EXPORT BinaryThresholdTool : public AutoSegmentationTool
  {
  public:

    Message3<double,double, bool> IntervalBordersChanged;
    Message1<double>     ThresholdingValueChanged;

    mitkClassMacro(BinaryThresholdTool, AutoSegmentationTool);
    itkNewMacro(BinaryThresholdTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

    virtual void Activated();
    virtual void Deactivated();

    virtual void SetThresholdValue(double value);
    virtual void AcceptCurrentThresholdValue(const std::string& organName, const Color& color);
    virtual void CancelThresholding();


  protected:

    BinaryThresholdTool(); // purposely hidden
    virtual ~BinaryThresholdTool();

    void SetupPreviewNodeFor( DataNode* nodeForThresholding );

    void CreateNewSegmentationFromThreshold(DataNode* node, const std::string& organType, const Color& color);

    void OnRoiDataChanged();

    template <typename TPixel, unsigned int VImageDimension>
    void ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep );

    DataNode::Pointer m_ThresholdFeedbackNode;
    DataNode::Pointer m_OriginalImageNode;
    DataNode::Pointer m_NodeForThresholding;

    double m_SensibleMinimumThresholdValue;
    double m_SensibleMaximumThresholdValue;
    double m_CurrentThresholdValue;
    bool m_IsFloatImage;

  };

} // namespace

#endif

