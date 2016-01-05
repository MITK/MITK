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


#include "mitkGizmo.h"
#include "mitkGizmoInteractor3D.h"

// MITK includes
#include <mitkRenderingManager.h>
#include <mitkBaseRenderer.h>
#include <mitkVtkInterpolationProperty.h>

// VTK includes
#include <vtkConeSource.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkAppendPolyData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPointData.h>
#include <vtkCharArray.h>
#include <vtkRenderWindow.h>

// ITK includes
#include <itkCommand.h>

// MicroServices
#include <usGetModuleContext.h>

mitk::DataNode::Pointer mitk::Gizmo::AddGizmoToNode(DataNode* node, DataStorage* storage)
{
  assert(node);
  if (node->GetData() == nullptr ||
      node->GetData()->GetGeometry() == nullptr)
  {
    return nullptr;
  }
  //--------------------------------------------------------------
  // Add visual gizmo that follows the node to be manipulated
  //--------------------------------------------------------------

  auto gizmo = Gizmo::New();
  auto gizmoNode = DataNode::New();
  gizmoNode->SetData(gizmo);
  gizmoNode->SetProperty("material.ambientCoefficient", FloatProperty::New(0));
  gizmoNode->SetProperty("material.diffuseCoefficient", FloatProperty::New(1));
  gizmoNode->SetProperty("material.specularCoefficient", FloatProperty::New(0.2));
  gizmoNode->SetProperty("material.interpolation", VtkInterpolationProperty::New(2)); // PHONG
  gizmoNode->SetProperty("scalar visibility", BoolProperty::New(true));
  gizmoNode->SetProperty("ScalarsRangeMinimum", DoubleProperty::New(0));
  gizmoNode->SetProperty("ScalarsRangeMaximum", DoubleProperty::New(2));

  // Hide by default, show in all 3D windows
  gizmoNode->SetProperty("helper object", BoolProperty::New(true));
  gizmoNode->SetProperty("visible", BoolProperty::New(false));
  auto rwList = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  for (auto& rw : rwList)
  {
    auto renderer = BaseRenderer::GetInstance(rw);
    if (renderer != nullptr)
    {
      if (renderer->GetMapperID() == BaseRenderer::Standard3D)
      {
        gizmoNode->SetProperty("visible", BoolProperty::New(true), renderer);
      }
    }
  }

  gizmo->FollowGeometry(node->GetData()->GetGeometry());

  //--------------------------------------------------------------
  // Add interaction to the gizmo
  //--------------------------------------------------------------

  mitk::GizmoInteractor3D::Pointer interactor = mitk::GizmoInteractor3D::New();
  interactor->LoadStateMachine("Gizmo3DStates.xml", us::GetModuleContext()->GetModule());
  //interactor->SetEventConfig("Gizmo3D.xml", us::ModuleRegistry::GetModule("MitkGizmo"));

  interactor->SetGizmoNode(gizmoNode);
  interactor->SetManipulatedObjectNode(node);

  assert(storage);
  storage->Add(gizmoNode);

  return gizmoNode;
}

mitk::Gizmo::Gizmo()
: Surface()
{
  m_Center.Fill(0);

  m_Center[0] = 58;
  m_Center[1] = 24;
  m_Center[2] = 0;

  m_AxisX.Fill(0); m_AxisX[0] = 1;
  m_AxisY.Fill(0); m_AxisY[1] = 1;
  m_AxisZ.Fill(0); m_AxisZ[2] = 1;

  m_Scaling.Fill(1);

  UpdateRepresentation();
}

void mitk::Gizmo::UpdateRepresentation()
{
  /* bounding box around the unscaled bounding object */
  ScalarType bounds[6]={-m_Scaling[0]*1.2,+m_Scaling[0]*1.2,
                        -m_Scaling[1]*1.2,+m_Scaling[1]*1.2,
                        -m_Scaling[2]*1.2,+m_Scaling[2]*1.2};
  GetGeometry()->SetBounds(bounds);
  GetTimeGeometry()->Update();

  SetVtkPolyData( BuildGizmo() );
}

mitk::Gizmo::~Gizmo()
{
}

namespace {

void AssignScalarValueTo(vtkPolyData* polydata, char value)
{
  vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();

  int numberOfPoints = polydata->GetNumberOfPoints();
  pointData->SetNumberOfComponents(1);
  pointData->SetNumberOfTuples(numberOfPoints);
  pointData->FillComponent(0,value);
  polydata->GetPointData()->SetScalars(pointData);
}

vtkSmartPointer<vtkPolyData> BuildAxis(const mitk::Point3D& center,
                                       const mitk::Vector3D& axis,
                                       double halflength,
                                       char vertexValueAxis,
                                       char vertexValueRing)
{
  const double shaftRadius = 1;
  const double arrowHeight = shaftRadius * 6;

  vtkSmartPointer<vtkAppendPolyData> appenderAxis = vtkSmartPointer<vtkAppendPolyData>::New();
  for (double sign = -1.0; sign < 3.0; sign += 2)
  {
    vtkSmartPointer<vtkConeSource> cone = vtkConeSource::New();
    cone->SetCenter(center[0] + sign * axis[0] * (halflength * 1.1 + arrowHeight * 0.5), // arrow tips at 110% of radius
                    center[1] + sign * axis[1] * (halflength * 1.1 + arrowHeight * 0.5),
                    center[2] + sign * axis[2] * (halflength * 1.1 + arrowHeight * 0.5));
    cone->SetDirection(sign * axis[0],
                       sign * axis[1],
                       sign * axis[2]);
    cone->SetRadius(shaftRadius * 3);
    cone->SetHeight(arrowHeight);
    cone->SetResolution(40);
    cone->CappingOn();
    cone->Update();
    appenderAxis->AddInputData(cone->GetOutput());
  }

  vtkSmartPointer<vtkCylinderSource> shaft = vtkCylinderSource::New();
  shaft->SetRadius(shaftRadius);
  shaft->SetHeight(halflength * 2.2);
  shaft->SetResolution(40);
  shaft->CappingOff();
  shaft->Update();

  vtkSmartPointer<vtkCylinderSource> ring = vtkCylinderSource::New();
  ring->SetRadius(halflength);
  ring->SetHeight(2 * shaftRadius);
  ring->SetResolution(100);
  ring->CappingOff();
  ring->Update();

  // TODO use Y axis and parameter axis, then calculate angle and turn around crossproduct axis
  vtkSmartPointer<vtkTransform> ringTrans = vtkSmartPointer<vtkTransform>::New();
  ringTrans->Translate(center[0], center[1], center[2]);
  if ( axis[0] > 0 ) // hmm... not really right
    ringTrans->RotateZ(90);
  if ( axis[2] > 0 )
    ringTrans->RotateX(90);

  vtkSmartPointer<vtkTransformPolyDataFilter> shaftTf =
      vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  shaftTf->SetInputConnection(shaft->GetOutputPort());
  shaftTf->SetTransform(ringTrans);
  shaftTf->Update();
  appenderAxis->AddInputData(shaftTf->GetOutput());
  appenderAxis->Update();
  AssignScalarValueTo(appenderAxis->GetOutput(), vertexValueAxis);

  vtkSmartPointer<vtkTransformPolyDataFilter> ringTf =
      vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  ringTf->SetInputConnection(ring->GetOutputPort());
  ringTf->SetTransform(ringTrans);
  ringTf->Update();
  AssignScalarValueTo( ringTf->GetOutput(), vertexValueRing );

  vtkSmartPointer<vtkAppendPolyData> appenderGlobal = vtkSmartPointer<vtkAppendPolyData>::New();
  appenderGlobal->AddInputData( appenderAxis->GetOutput() );
  appenderGlobal->AddInputData( ringTf->GetOutput() );
  appenderGlobal->Update();

  vtkSmartPointer<vtkPolyData> result = appenderGlobal->GetOutput();
  return result;
}

} // unnamed namespace

vtkSmartPointer<vtkPolyData> mitk::Gizmo::BuildGizmo()
{
  double longestAxis = std::max( m_Scaling[0], m_Scaling[1] );
  longestAxis = std::max( longestAxis, m_Scaling[2] );

  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();
  appender->AddInputData( BuildAxis(m_Center, m_AxisX, longestAxis, MoveAlongAxisX, RotateAroundAxisX) );
  appender->AddInputData( BuildAxis(m_Center, m_AxisY, longestAxis, MoveAlongAxisY, RotateAroundAxisY) );
  appender->AddInputData( BuildAxis(m_Center, m_AxisZ, longestAxis, MoveAlongAxisZ, RotateAroundAxisZ) );

  auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(m_Center[0], m_Center[1], m_Center[2]);
  sphereSource->SetRadius(longestAxis * 0.05);
  sphereSource->Update();
  AssignScalarValueTo( sphereSource->GetOutput(), MoveFreely );
  
  appender->AddInputData( sphereSource->GetOutput() );

  appender->Update();
  return appender->GetOutput();
}

void mitk::Gizmo::FollowGeometry(BaseGeometry* geom)
{
  auto observer = itk::SimpleMemberCommand<Gizmo>::New();
  observer->SetCallbackFunction(this, &Gizmo::OnFollowedGeometryModified);

  m_FollowedGeometry = geom;
  m_FollowedGeometry->AddObserver(itk::ModifiedEvent(), observer);

  // initial adjustment
  OnFollowedGeometryModified();
}

void mitk::Gizmo::OnFollowedGeometryModified()
{
    m_Center = m_FollowedGeometry->GetCenter();

    m_AxisX = m_FollowedGeometry->GetAxisVector(0);
    m_AxisY = m_FollowedGeometry->GetAxisVector(1);
    m_AxisZ = m_FollowedGeometry->GetAxisVector(2);

    m_AxisX.Normalize();
    m_AxisY.Normalize();
    m_AxisZ.Normalize();

    for ( int dim = 0; dim < 3; ++dim )
    {
      m_Scaling[dim] = 0.5 * m_FollowedGeometry->GetExtentInMM(dim);
    }

    UpdateRepresentation();
}

mitk::Gizmo::HandleType mitk::Gizmo::GetHandleFromPointID(vtkIdType id)
{

#define CheckHandleType(type) \
  if (static_cast<int>(dataValue) == static_cast<int>(type)) \
    return type;

  assert(GetVtkPolyData());
  assert(GetVtkPolyData()->GetPointData());
  assert(GetVtkPolyData()->GetPointData()->GetScalars());
  // TODO check if id exists.. what otherwise?
  double dataValue = GetVtkPolyData()->GetPointData()->GetScalars()->GetTuple1(id);

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  return NoHandle;
}

std::string mitk::Gizmo::HandleTypeToString(HandleType type)
{
#define CheckHandleType(candidateType) \
  if (type == candidateType) \
    return std::string(#candidateType);

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  CheckHandleType(NoHandle);
  return "InvalidHandleType";
}
