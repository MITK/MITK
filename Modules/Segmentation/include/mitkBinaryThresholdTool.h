/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryThresholdTool_h
#define mitkBinaryThresholdTool_h

#include <mitkBinaryThresholdBaseTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
   * \brief Single-threshold segmentation tool for interactive segmentation.
   *
   * Segments an image by applying a single lower threshold. All pixels above
   * the threshold value are included in the segmentation. The upper threshold
   * is locked to the maximum image value.
   *
   * \ingroup ToolManagerEtAl
   * \sa BinaryThresholdBaseTool, BinaryThresholdULTool
   */
  class MITKSEGMENTATION_EXPORT BinaryThresholdTool : public BinaryThresholdBaseTool
  {
  public:

    mitkClassMacro(BinaryThresholdTool, BinaryThresholdBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Returns the icon resource for this tool's button. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Returns the display name of this tool ("Threshold"). */
    const char *GetName() const override;

    /**
     * \brief Sets the single threshold value.
     * \param[in] value The threshold value. Pixels at or above this value are segmented.
     */
    virtual void SetThresholdValue(double value);

  protected:
    BinaryThresholdTool(); // purposely hidden
    ~BinaryThresholdTool() override;
  };

} // namespace

#endif
