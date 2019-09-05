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
#include <MitkSegmentationExports.h>
#include "mitkAutoSegmentationTool.h"
#include "mitkDataNode.h"
#include <mitkImage.h>

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
  class MITKSEGMENTATION_EXPORT BinaryThresholdULTool : public AutoSegmentationTool
  {
  public:

    Message3<double,double,bool> IntervalBordersChanged;
    Message2<mitk::ScalarType, mitk::ScalarType>     ThresholdingValuesChanged;

    mitkClassMacro(BinaryThresholdULTool, AutoSegmentationTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    virtual const char* GetName() const override;

    virtual void Activated() override;
    virtual void Deactivated() override;

    virtual void SetThresholdValues(double lower, double upper);
    virtual void AcceptCurrentThresholdValue();
    virtual void CancelThresholding();


  protected:

    BinaryThresholdULTool(); // purposely hidden
    virtual ~BinaryThresholdULTool();

    virtual void SetupPreviewNode();
    virtual void runItkThreshold(Image::Pointer imageToThreshold, Image::Pointer previewImage);
    virtual void updateThresholdValue();

    void UpdatePreview();
    void CreateNewSegmentationFromThreshold(DataNode* node);
    void OnRoiDataChanged();

    DataNode::Pointer m_ThresholdFeedbackNode;
    DataNode::Pointer m_OriginalImageNode;
    DataNode::Pointer m_NodeForThresholding;

    mitk::ScalarType m_SensibleMinimumThresholdValue;
    mitk::ScalarType m_SensibleMaximumThresholdValue;
    mitk::ScalarType m_CurrentLowerThresholdValue;
    mitk::ScalarType m_CurrentUpperThresholdValue;

    template <typename TPixel, unsigned int VImageDimension>
    void ITKThresholding(
      itk::Image<TPixel,
      VImageDimension>* originalImage,
      mitk::Image::Pointer segmentation,
      double lower,
      double upper
    );
  };

} // namespace

#endif

