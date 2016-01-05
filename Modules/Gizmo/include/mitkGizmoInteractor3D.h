/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkGizmoInteractor3D_h
#define mitkGizmoInteractor3D_h

#include "mitkGizmo.h"

// MITK includes
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>
#include <mitkDataNode.h>

// VTK includes
#include <vtkPointPicker.h>

#include "MitkGizmoExports.h"

namespace mitk
{

class MITKGIZMO_EXPORT GizmoInteractor3D: public DataInteractor
{
public:

  mitkClassMacro(GizmoInteractor3D, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void SetGizmoNode(DataNode* node);
  void SetManipulatedObjectNode(DataNode* node);

protected:

  GizmoInteractor3D();
  virtual ~GizmoInteractor3D();

  virtual void ConnectActionsAndFunctions() override;

  bool PickHandle(const InteractionEvent*);

private:

    Gizmo::Pointer m_Gizmo;
    DataNode::Pointer m_ManipulatedObject;

    vtkSmartPointer<vtkPointPicker> m_PointPicker;
};

}
#endif
