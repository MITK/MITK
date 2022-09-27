/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFillRegionBaseTool_h_Included
#define mitkFillRegionBaseTool_h_Included

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
    * You may reimplement this function to change the strategy to determin the fill region.*/
    virtual Image::Pointer GenerateFillImage(const Image* workingSlice, Point3D seedPoint) const;

    /** Function that is called by OnClick before the filling is executed. If you want to do special
    * preperation (e.g. change m_FillValue, you can overwrite this function. */
    virtual void PrepareFilling(const Image* workingSlice, Point3D seedPoint) = 0;

    Label::PixelType m_FillValue = 0;
    MultiLabelSegmentation::MergeStyle m_MergeStyle = MultiLabelSegmentation::MergeStyle::Replace;
  private:
  };

} // namespace

#endif
