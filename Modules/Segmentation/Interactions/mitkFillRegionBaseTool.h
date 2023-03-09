/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFillRegionBaseTool_h
#define mitkFillRegionBaseTool_h

#include "mitkCommon.h"
#include "mitkContourModelUtils.h"
#include "mitkContourUtils.h" //TODO remove legacy support
#include "mitkImage.h"
#include "mitkSegTool2D.h"
#include <MitkSegmentationExports.h>

#include "mitkDataNode.h"

#include "mitkImageCast.h"

namespace mitk
{
  /**
    \brief Base class for tools that fill a connected region of a 2D slice

    \sa Tool

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.

    $Author: nolden $
  */
  class MITKSEGMENTATION_EXPORT FillRegionBaseTool : public SegTool2D
  {
  public:
    mitkClassMacro(FillRegionBaseTool, SegTool2D);

  protected:
    FillRegionBaseTool();             // purposely hidden
    ~FillRegionBaseTool() override;

    void ConnectActionsAndFunctions() override;

    /// \brief Add a control point and finish current segment.
    virtual void OnClick(StateMachineAction*, InteractionEvent* interactionEvent);

    /** Function that generates the mask image that indicates which pixels should be filled.
    * Caller of this function assumes that all pixels that should be filled have the value 1.
    * Pixels that should stay untouched should have the value 0.
    * The default implementation marks the connected reagion around seedPoint, that has
    * the same pixel value/label like the seedPoint.
    * You may reimplement this function to change the strategy to determin the fill region.
    * @param workingSlice part of the segmentation image that should be used to determin the fill image.
    * @param seedPoint The world coordinate position where the user has cliced.
    * @param [out] seedLabelValue The function should return the label value that should be assumed
    * as clicked on, given the seedPoint.
    * @return Return the image maske that indicates which pixels should be filled. Returning
    * a null pointer indicates that there is nothing to fill.
    */
    virtual Image::Pointer GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const;

    /** Function that is called by OnClick before the filling is executed. If you want to do special
    * preperation (e.g. change m_FillLabelValue, you can overwrite this function. */
    virtual void PrepareFilling(const Image* workingSlice, Point3D seedPoint) = 0;

    Label::PixelType m_FillLabelValue = 0;
    Label::PixelType m_SeedLabelValue = 0;

    MultiLabelSegmentation::MergeStyle m_MergeStyle = MultiLabelSegmentation::MergeStyle::Replace;
  private:
  };

} // namespace

#endif
