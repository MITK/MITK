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
    void OnRenderWindowClicked(StateMachineAction *, InteractionEvent *);
    void OnDelete(StateMachineAction *, InteractionEvent *);
    std::stringstream GetPointsAsCSVString(const mitk::BaseGeometry *);

  protected:
    MedSAMTool() = default;
    ~MedSAMTool() = default;

  private:
    void CreateBoundingShapeInteractor(bool);
    mitk::Geometry3D::Pointer InitializeWithImageGeometry(const mitk::BaseGeometry *) const;
    DataNode::Pointer m_BoundingBoxNode;
    BoundingShapeInteractor::Pointer m_BoundingShapeInteractor;
  };
} // namespace

#endif
