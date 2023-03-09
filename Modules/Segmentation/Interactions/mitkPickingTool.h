/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPickingTool_h
#define mitkPickingTool_h

#include "mitkSegWithPreviewTool.h"
#include "mitkPointSet.h"

#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
  \brief Extracts a single region from a segmentation image and creates a new image with same geometry of the input
  image.

  The region is extracted in 3D space. This is done by performing region growing within the desired region.
  Use shift click to add the seed point.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT PickingTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(PickingTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    /**Clears all picks and updates the preview.*/
    void ClearPicks();

    bool HasPicks() const;

  protected:
    PickingTool(); // purposely hidden
    ~PickingTool() override;

    void ConnectActionsAndFunctions() override;

    /// \brief Add point action of StateMachine pattern
    virtual void OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent);

    /// \brief Delete action of StateMachine pattern
    virtual void OnDelete(StateMachineAction*, InteractionEvent* interactionEvent);

    /// \brief Clear all seed points.
    void ClearSeeds();

    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;

    // seed point
    PointSet::Pointer m_PointSet;
    DataNode::Pointer m_PointSetNode;
  };

} // namespace

#endif
