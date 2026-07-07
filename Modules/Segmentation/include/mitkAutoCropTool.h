/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAutoCropTool_h
#define mitkAutoCropTool_h

#include <mitkCommon.h>
#include <mitkSegmentationsProcessingTool.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
    \brief Crops selected segmentations to the smallest bounding box enclosing non-zero pixels.

    \ingroup ToolManagerEtAl
    \sa mitk::Tool
    \sa QmitkInteractiveSegmentation

    Uses AutoCropImageFilter internally to determine the minimal bounding box
    around the foreground region (non-zero pixels) and replaces the node data
    with the cropped result.
  */
  class MITKSEGMENTATION_EXPORT AutoCropTool : public SegmentationsProcessingTool
  {
  public:
    mitkClassMacro(AutoCropTool, SegmentationsProcessingTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the human-readable name of this tool ("Crop"). */
    const char *GetName() const override;

  protected:
    /** \brief Crop a single working data node to its foreground bounding box.
      \return True on success, false if the node is null or processing fails.
    */
    bool ProcessOneWorkingData(DataNode *node) override;

    /** \brief Return a user-facing error message prefix for failed crop operations. */
    std::string GetErrorMessage() override;

    AutoCropTool(); // purposely hidden
    ~AutoCropTool() override;
  };

} // namespace

#endif
