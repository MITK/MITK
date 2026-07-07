/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryThresholdULTool_h
#define mitkBinaryThresholdULTool_h

#include <mitkBinaryThresholdBaseTool.h>
#include <MitkSegmentationExports.h>


namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
   * \brief Upper/lower dual-threshold segmentation tool for interactive segmentation.
   *
   * Segments an image by applying both a lower and an upper threshold. Only pixels
   * within the [lower, upper] range are included in the segmentation result.
   *
   * \ingroup ToolManagerEtAl
   * \sa BinaryThresholdBaseTool, BinaryThresholdTool
   */
  class MITKSEGMENTATION_EXPORT BinaryThresholdULTool : public BinaryThresholdBaseTool
  {
  public:
    mitkClassMacro(BinaryThresholdULTool, BinaryThresholdBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns the icon resource for this tool's button. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Returns the display name of this tool ("UL Threshold"). */
    const char *GetName() const override;

  protected:
    BinaryThresholdULTool(); // purposely hidden
    ~BinaryThresholdULTool() override;
  };

} // namespace

#endif
