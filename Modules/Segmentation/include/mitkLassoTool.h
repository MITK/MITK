/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLassoTool_h
#define mitkLassoTool_h

#include <mitkEditableContourTool.h>
#include <mitkContourTool.h>
#include <mitkContourModelInteractor.h>

namespace mitk
{
  /**
    \brief A 2D segmentation tool to draw polygon structures.

    The contour between the last point and the current mouse position is
    computed by forming a straight line.

    The tool always assumes that unconfirmed contours are always defined for the
    current time point. So the time step in which the contours will be stored as
    segmentations will be determined when the contours got confirmed. Then they
    will be transferred to the slices of the currently selected time step.

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

    /** \brief Return the cursor icon resource for this tool. */
    us::ModuleResource GetCursorIconResource() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Return the human-readable name of this tool ("Lasso"). */
    const char *GetName() const override;

  protected:
    LassoTool();
    ~LassoTool() override;

    /** \brief Wire state machine actions to the corresponding member functions. */
    void ConnectActionsAndFunctions() override;

    /** \brief Finalize the contour and write it back as a segmentation result. */
    void FinishTool() override;

  private:
    mitk::ContourModelInteractor::Pointer m_ContourInteractor;
  };
}

#endif
