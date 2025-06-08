/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMedSAMTool_h
#define mitkMedSAMTool_h

#include "mitkSegmentAnythingTool.h"
#include "mitkBoundingShapeInteractor.h"
#include <MitkSegmentationExports.h>
#include <mitkRenderingManager.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
  \brief Medical Segment Anything Model interactive 2D tool class.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT MedSAMTool : public SegmentAnythingTool
  {
  public:
    mitkClassMacro(MedSAMTool, SegmentAnythingTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;

    void Activated() override;
    void Deactivated() override;
    bool HasPicks() const override;
    void ClearPicks() override;
    void ConnectActionsAndFunctions() override;
    std::string GetPointsAsCSVString(const mitk::BaseGeometry *baseGeometry) const override;

    /**
     * @brief Adds bounding box in the render window when clicked.
     * 
     */
    void OnRenderWindowClicked(StateMachineAction *, InteractionEvent *interactionEvent);

    /**
     * @brief Deletes bounding box from the render window.
     * 
     */
    void OnDelete(StateMachineAction *, InteractionEvent *);

    void OnMove(StateMachineAction *, InteractionEvent *);
    void OnRelease(StateMachineAction *, InteractionEvent *);
    void OnPrimaryButtonPressed(StateMachineAction *, InteractionEvent *);

  protected:
    MedSAMTool() = default;
    ~MedSAMTool() = default;

  private:
    /**
     * @brief Initializes the Bounding Shape Interactor object
     * 
     */
    void CreateBoundingShapeInteractor(bool rotationEnabled);

    /**
     * @brief initializes a new bounding shape using the selected image geometry.
     * 
     */
    mitk::Geometry3D::Pointer InitializeWithImageGeometry(const mitk::BaseGeometry *geometry) const;
    DataNode::Pointer m_BoundingBoxNode;
    BoundingShapeInteractor::Pointer m_BoundingShapeInteractor;
  };
}
#endif
