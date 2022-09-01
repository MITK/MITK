/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLassoTool_h
#define mitkLassoTool_h

#include "mitkEditableContourTool.h"
#include "mitkContourTool.h"
#include "mitkContourModelInteractor.h"

namespace mitk
{
  /**
    \brief A 2D segmentation tool to draw polygon structures.

    The contour between the last point and the current mouse position is
    computed by forming a straight line.

    The tool always assumes that unconfirmed contours are always defined for the
    current time point. So the time step in which the contours will be stored as
    segmentations will be determined when the contours got confirmed. Then they
    will be transfered to the slices of the currently selected time step.

    \sa SegTool2D

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT LassoTool : public EditableContourTool
  {
  public:
    mitkClassMacro(LassoTool, SegTool2D);
    itkFactorylessNewMacro(Self);

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;
    const char *GetName() const override;
    const char **GetXPM() const override;

  protected:
    LassoTool();
    ~LassoTool() override;

    void ConnectActionsAndFunctions() override;
    void FinishTool() override;

  private:
    mitk::ContourModelInteractor::Pointer m_ContourInteractor;
  };
}

#endif
