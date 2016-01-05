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

#include "mitkGizmoInteractor3D.h"

#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkRotationOperation.h>
#include <mitkScaleOperation.h>
#include <mitkSurface.h>
#include <mitkVtkMapper.h>

#include <vtkCamera.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>

mitk::GizmoInteractor3D::GizmoInteractor3D()
{
    m_PointPicker = vtkSmartPointer<vtkPointPicker>::New();
    m_PointPicker->SetTolerance(0.005);
}

mitk::GizmoInteractor3D::~GizmoInteractor3D()
{
}

void mitk::GizmoInteractor3D::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually executing an action
  CONNECT_CONDITION("pickedHandle", PickHandle);

  // **Function** in the statmachine patterns also referred to as **Actions**
  //CONNECT_FUNCTION("pickHandle",PickHandle);
}

void mitk::GizmoInteractor3D::SetGizmoNode(DataNode* node)
{
  // the gizmo
  DataInteractor::SetDataNode(node);

  m_Gizmo = dynamic_cast<mitk::Gizmo*>(node->GetData());

  m_PointPicker->GetPickList()->RemoveAllItems();
  m_PointPicker->PickFromListOff();
}

void mitk::GizmoInteractor3D::SetManipulatedObjectNode(DataNode* node)
{
  m_ManipulatedObject = node;
}


bool mitk::GizmoInteractor3D::PickHandle(const InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  DataNode::Pointer gizmoNode = this->GetDataNode();

  if (m_Gizmo.IsNull())
    return false;

  if (m_ManipulatedObject.IsNull())
    return false;

  if (interactionEvent->GetSender()->GetRenderWindow()->GetNeverRendered())
    return false;

  if ( !m_PointPicker->GetPickFromList() )
  {
    auto mapper = GetDataNode()->GetMapper(mitk::BaseRenderer::Standard3D);
    auto vtk_mapper = dynamic_cast<VtkMapper*>(mapper);
    if ( vtk_mapper )
    { // doing this each time is bizarre
      m_PointPicker->AddPickList(vtk_mapper->GetVtkProp(interactionEvent->GetSender()));
      m_PointPicker->PickFromListOn();
    }
  }

  auto displayPosition = positionEvent->GetPointerPositionOnScreen();
  m_PointPicker->Pick( displayPosition[0], displayPosition[1], 0,
                       interactionEvent->GetSender()->GetVtkRenderer() );

  vtkIdType pickedPointID = m_PointPicker->GetPointId();
  if (pickedPointID == -1)
  {
      return false;
  }

  // _something_ picked
  auto handleType = m_Gizmo->GetHandleFromPointID(pickedPointID);
  std::cout << "Picked pID " << pickedPointID 
            << " --> " << Gizmo::HandleTypeToString(handleType) << std::endl;

  return true;
}
