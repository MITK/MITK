/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITK_AUTO_ML_SEGMENTATION_WITH_PREVIEW_TOOL_H
#define MITK_AUTO_ML_SEGMENTATION_WITH_PREVIEW_TOOL_H

#include "mitkAutoSegmentationWithPreviewTool.h"
#include "mitkDataNode.h"
#include "mitkLabelSetImage.h"

#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
  \brief Base class for any auto segmentation tool that provides a preview of the new segmentation and generates
  segmentations with multiple labels.

  This tool class implements the basic logic to handle previews of multi label segmentations and
  to allow to pick arbitrary labels as selected and merge them to a single segmentation to store this
  segmentation as confirmed segmentation.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation
  */
  class MITKSEGMENTATION_EXPORT AutoMLSegmentationWithPreviewTool : public AutoSegmentationWithPreviewTool
  {
  public:
    mitkClassMacro(AutoMLSegmentationWithPreviewTool, AutoSegmentationWithPreviewTool);

    void Activated() override;
    void Deactivated() override;

    using SelectedLabelVectorType = std::vector<Label::PixelType>;
    void SetSelectedLabels(const SelectedLabelVectorType& regions);
    SelectedLabelVectorType GetSelectedLabels() const;

    const LabelSetImage* GetMLPreview() const;

  protected:
    AutoMLSegmentationWithPreviewTool();
    ~AutoMLSegmentationWithPreviewTool() = default;

    void UpdateCleanUp() override;
    void DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep) override;

    /** Function to generate the new multi lable preview for a given time step input image.
     * The function must be implemented by derived tools.
     * This function is called by DoUpdatePreview if needed.
     * Reasons are:
     * - ML preview does not exist
     * - Modify time of tools is newer then of ML preview
     * - ML preview was not generated for the current selected timestep of input image or for the current selected timepoint.*/
    virtual LabelSetImage::Pointer ComputeMLPreview(const Image* inputAtTimeStep, TimeStepType timeStep) = 0;

  private:
    /** Function to generate a simple (single lable) preview by merging all labels of the ML preview that are selected and
     * copies that single label preview to the passed previewImage.
     * This function is called by DoUpdatePreview if needed.
     * @param mlPreviewImage Multi label preview that is the source.
     * @param previewImage Pointer to the single label preview image that should receive the merged selected labels.
     * @timeStep Time step of the previewImage that should be filled.*/
    template <typename TPixel, unsigned int VImageDimension>
    void CalculateMergedSimplePreview(const itk::Image<TPixel, VImageDimension>* mlImage, mitk::Image* segmentation, unsigned int timeStep);

    SelectedLabelVectorType m_SelectedLabels = {};

    // holds the multilabel result as a preview image
    mitk::DataNode::Pointer m_MLPreviewNode;
    TimeStepType m_LastMLTimeStep = 0;
  };
}
#endif
