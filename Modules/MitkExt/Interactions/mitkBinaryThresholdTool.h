/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
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
#include "mitkAutoSegmentationTool.h"
#include "mitkDataTreeNode.h"

#include <itkImage.h>

namespace mitk
{

/**
  \brief Calculates the segmented volumes for binary images.

  \ingroup Reliver
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
*/
class MITKEXT_CORE_EXPORT BinaryThresholdTool : public AutoSegmentationTool
{
  public:

    Message2<int,int> IntervalBordersChanged;
    Message1<int>     ThresholdingValueChanged;
    
    mitkClassMacro(BinaryThresholdTool, AutoSegmentationTool);
    itkNewMacro(BinaryThresholdTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    
    virtual void Activated();
    virtual void Deactivated();

    virtual void SetThresholdValue(int value);
    virtual void AcceptCurrentThresholdValue(const std::string& organType, const std::string& organName);

  protected:
    
    BinaryThresholdTool(); // purposely hidden
    virtual ~BinaryThresholdTool();

    void SetupPreviewNodeFor( DataTreeNode* nodeForThresholding );

    void CreateNewSegmentationFromThreshold(DataTreeNode* node, const std::string& organType, const std::string& organName);

    template <typename TPixel, unsigned int VImageDimension>
    void ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep );

    DataTreeNode::Pointer m_ThresholdFeedbackNode;
    DataTreeNode::Pointer m_NodeForThresholding;

    int m_SensibleMinimumThresholdValue;
    int m_SensibleMaximumThresholdValue;
    int m_CurrentThresholdValue;
};

} // namespace

#endif

