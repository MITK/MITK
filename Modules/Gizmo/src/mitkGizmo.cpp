/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGizmo.h"
#include "mitkGizmoInteractor.h"

// MITK includes
#include <mitkBaseRenderer.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkRenderingManager.h>
#include <mitkVtkInterpolationProperty.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCharArray.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyDataNormals.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTubeFilter.h>

// ITK includes
#include <itkCommand.h>

// MicroServices
#include <usGetModuleContext.h>

namespace
{
  const char *PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY = "gizmo.originalObjectOpacity";
}

namespace mitk
{
  //! Private object, removing the Gizmo from data storage along with its manipulated object.
  class GizmoRemover
  {
  public:
    GizmoRemover() : m_Storage(nullptr), m_GizmoNode(nullptr), m_ManipulatedNode(nullptr), m_StorageObserverTag(0) {}
    //! Tell the object about the storage that contains both the nodes
    //! containing the gizmo and the manipulated object.
    //!
    //! The method sets up observation of
    //! - removal of the manipulated node from storage
    //! - destruction of storage itself
    void UpdateStorageObservation(mitk::DataStorage *storage,
                                  mitk::DataNode *gizmo_node,
                                  mitk::DataNode *manipulated_node)
    {
      if (m_Storage != nullptr)
      {
        m_Storage->RemoveNodeEvent.RemoveListener(
          mitk::MessageDelegate1<GizmoRemover, const mitk::DataNode *>(this, &GizmoRemover::OnDataNodeHasBeenRemoved));
        m_Storage->RemoveObserver(m_StorageObserverTag);
      }

      m_Storage = storage;
      m_GizmoNode = gizmo_node;
      m_ManipulatedNode = manipulated_node;

      if (m_Storage != nullptr)
      {
        m_Storage->RemoveNodeEvent.AddListener(
          mitk::MessageDelegate1<GizmoRemover, const mitk::DataNode *>(this, &GizmoRemover::OnDataNodeHasBeenRemoved));

        itk::SimpleMemberCommand<GizmoRemover>::Pointer command = itk::SimpleMemberCommand<GizmoRemover>::New();
        command->SetCallbackFunction(this, &mitk::GizmoRemover::OnDataStorageDeleted);
        m_StorageObserverTag = m_Storage->AddObserver(itk::ModifiedEvent(), command);
      }
    }

    //! Callback notified on destruction of DataStorage
    void OnDataStorageDeleted() { m_Storage = nullptr; }
    //! Callback notified on removal of _any_ object from data storage
    void OnDataNodeHasBeenRemoved(const mitk::DataNode *node)
    {
      if (node == m_ManipulatedNode)
      {
        // m_Storage is still alive because it is the emitter
        if (m_Storage->Exists(m_GizmoNode))
        {
          m_Storage->Remove(m_GizmoNode);
          // normally, gizmo will be deleted here (unless somebody
          // still holds a reference to it)
        }
      }
    }

    //! Clean up our observer registrations
    ~GizmoRemover()
    {
      if (m_Storage)
      {
        m_Storage->RemoveNodeEvent.RemoveListener(
          mitk::MessageDelegate1<GizmoRemover, const mitk::DataNode *>(this, &GizmoRemover::OnDataNodeHasBeenRemoved));
        m_Storage->RemoveObserver(m_StorageObserverTag);
      }
    }

  private:
    mitk::DataStorage *m_Storage;
    mitk::DataNode *m_GizmoNode;
    mitk::DataNode *m_ManipulatedNode;
    unsigned long m_StorageObserverTag;
  };

} // namespace MITK

bool mitk::Gizmo::HasGizmoAttached(DataNode *node, DataStorage *storage)
{
  auto typeCondition = TNodePredicateDataType<Gizmo>::New();
  auto gizmoChildren = storage->GetDerivations(node, typeCondition);
  return !gizmoChildren->empty();
}

bool mitk::Gizmo::RemoveGizmoFromNode(DataNode *node, DataStorage *storage)
{
  if (node == nullptr || storage == nullptr)
  {
    return false;
  }

  auto typeCondition = TNodePredicateDataType<Gizmo>::New();
  auto gizmoChildren = storage->GetDerivations(node, typeCondition);

  for (auto &gizmoChild : *gizmoChildren)
  {
    auto *gizmo = dynamic_cast<Gizmo *>(gizmoChild->GetData());
    if (gizmo)
    {
      storage->Remove(gizmoChild);
      gizmo->m_GizmoRemover->UpdateStorageObservation(nullptr, nullptr, nullptr);
    }
  }

  //--------------------------------------------------------------
  // Restore original opacity if we changed it
  //--------------------------------------------------------------
  float originalOpacity = 1.0;
  if (node->GetFloatProperty(PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY, originalOpacity))
  {
    node->SetOpacity(originalOpacity);
    node->GetPropertyList()->DeleteProperty(PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY);
  }

  return !gizmoChildren->empty();
}

mitk::DataNode::Pointer mitk::Gizmo::AddGizmoToNode(DataNode *node, DataStorage *storage)
{
  assert(node);
  if (node->GetData() == nullptr || node->GetData()->GetGeometry() == nullptr)
  {
    return nullptr;
  }
  //--------------------------------------------------------------
  // Add visual gizmo that follows the node to be manipulated
  //--------------------------------------------------------------

  auto gizmo = Gizmo::New();
  auto gizmoNode = DataNode::New();
  gizmoNode->SetName("Gizmo");
  gizmoNode->SetData(gizmo);
  gizmo->FollowGeometry(node->GetData()->GetGeometry());

  //--------------------------------------------------------------
  // Add interaction to the gizmo
  //--------------------------------------------------------------

  mitk::GizmoInteractor::Pointer interactor = mitk::GizmoInteractor::New();
  interactor->LoadStateMachine("Gizmo3DStates.xml", us::GetModuleContext()->GetModule());
  interactor->SetEventConfig("Gizmo3DConfig.xml", us::ModuleRegistry::GetModule("MitkGizmo"));

  interactor->SetGizmoNode(gizmoNode);
  interactor->SetManipulatedObjectNode(node);

  //--------------------------------------------------------------
  // Note current opacity for later restore and lower it
  //--------------------------------------------------------------

  float currentNodeOpacity = 1.0;
  if (node->GetOpacity(currentNodeOpacity, nullptr))
  {
    if (currentNodeOpacity > 0.5f)
    {
      node->SetFloatProperty(PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY, currentNodeOpacity);
      node->SetOpacity(0.5f);
    }
  }

  if (storage)
  {
    storage->Add(gizmoNode, node);
    gizmo->m_GizmoRemover->UpdateStorageObservation(storage, gizmoNode, node);
  }

  return gizmoNode;
}

mitk::Gizmo::Gizmo()
  : Surface(), m_AllowTranslation(true), m_AllowRotation(true), m_AllowScaling(true), m_GizmoRemover(new GizmoRemover())
{
  m_Center.Fill(0);

  m_AxisX.Fill(0);
  m_AxisX[0] = 1;
  m_AxisY.Fill(0);
  m_AxisY[1] = 1;
  m_AxisZ.Fill(0);
  m_AxisZ[2] = 1;

  m_Radius.Fill(1);

  UpdateRepresentation();
}

mitk::Gizmo::~Gizmo()
{
  if (m_FollowedGeometry.IsNotNull())
  {
    m_FollowedGeometry->RemoveObserver(m_FollowerTag);
  }
}

void mitk::Gizmo::UpdateRepresentation()
{
  /* bounding box around the unscaled bounding object */
  ScalarType bounds[6] = {-m_Radius[0] * 1.2,
                          +m_Radius[0] * 1.2,
                          -m_Radius[1] * 1.2,
                          +m_Radius[1] * 1.2,
                          -m_Radius[2] * 1.2,
                          +m_Radius[2] * 1.2};
  GetGeometry()->SetBounds(bounds);
  GetTimeGeometry()->Update();

  SetVtkPolyData(BuildGizmo());
}

namespace
{
  void AssignScalarValueTo(vtkPolyData *polydata, char value)
  {
    vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();

    int numberOfPoints = polydata->GetNumberOfPoints();
    pointData->SetNumberOfComponents(1);
    pointData->SetNumberOfTuples(numberOfPoints);
    pointData->FillComponent(0, value);
    polydata->GetPointData()->SetScalars(pointData);
  }

  vtkSmartPointer<vtkPolyData> BuildAxis(const mitk::Point3D &center,
                                         const mitk::Vector3D &axis,
                                         double halflength,
                                         bool drawRing,
                                         char vertexValueAxis,
                                         char vertexValueRing,
                                         char vertexValueScale)
  {
    // Define all sizes relative to absolute size (thus that the gizmo will appear
    // in the same relative size for huge (size >> 1) and tiny (size << 1) objects).
    // This means that the gizmo will appear very different when a scene contains _both_
    // huge and tiny objects at the same time, but when the users zooms in on his
    // object of interest, the gizmo will always have the same relative size.
    const double shaftRadius = halflength * 0.02;
    const double arrowHeight = shaftRadius * 6;
    const int tubeSides = 15;

    // poly data appender to collect cones and tube that make up the axis
    vtkSmartPointer<vtkAppendPolyData> axisSource = vtkSmartPointer<vtkAppendPolyData>::New();

    // build two cones at the end of axis
    for (double sign = -1.0; sign < 3.0; sign += 2)
    {
      vtkSmartPointer<vtkConeSource> cone = vtkConeSource::New();
      // arrow tips at 110% of radius
      cone->SetCenter(center[0] + sign * axis[0] * (halflength * 1.1 + arrowHeight * 0.5),
                      center[1] + sign * axis[1] * (halflength * 1.1 + arrowHeight * 0.5),
                      center[2] + sign * axis[2] * (halflength * 1.1 + arrowHeight * 0.5));
      cone->SetDirection(sign * axis[0], sign * axis[1], sign * axis[2]);
      cone->SetRadius(shaftRadius * 3);
      cone->SetHeight(arrowHeight);
      cone->SetResolution(tubeSides);
      cone->CappingOn();
      cone->Update();
      AssignScalarValueTo(cone->GetOutput(), vertexValueScale);
      axisSource->AddInputData(cone->GetOutput());
    }

    // build the axis itself (as a tube around the line defining the axis)
    vtkSmartPointer<vtkPolyData> shaftSkeleton = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> shaftPoints = vtkSmartPointer<vtkPoints>::New();
    shaftPoints->InsertPoint(0, (center - axis * halflength * 1.1).GetDataPointer());
    shaftPoints->InsertPoint(1, (center + axis * halflength * 1.1).GetDataPointer());
    shaftSkeleton->SetPoints(shaftPoints);

    vtkSmartPointer<vtkCellArray> shaftLines = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType shaftLinePoints[] = {0, 1};
    shaftLines->InsertNextCell(2, shaftLinePoints);
    shaftSkeleton->SetLines(shaftLines);

    vtkSmartPointer<vtkTubeFilter> shaftSource = vtkSmartPointer<vtkTubeFilter>::New();
    shaftSource->SetInputData(shaftSkeleton);
    shaftSource->SetNumberOfSides(tubeSides);
    shaftSource->SetVaryRadiusToVaryRadiusOff();
    shaftSource->SetRadius(shaftRadius);
    shaftSource->Update();
    AssignScalarValueTo(shaftSource->GetOutput(), vertexValueAxis);

    axisSource->AddInputData(shaftSource->GetOutput());
    axisSource->Update();

    vtkSmartPointer<vtkTubeFilter> ringSource; // used after if block, so declare it here
    if (drawRing)
    {
      // build the ring orthogonal to the axis (as another tube)
      vtkSmartPointer<vtkPolyData> ringSkeleton = vtkSmartPointer<vtkPolyData>::New();
      vtkSmartPointer<vtkPoints> ringPoints = vtkSmartPointer<vtkPoints>::New();
      ringPoints->SetDataTypeToDouble(); // just some decision (see cast below)
      unsigned int numberOfRingPoints = 100;
      vtkSmartPointer<vtkCellArray> ringLines = vtkSmartPointer<vtkCellArray>::New();
      ringLines->InsertNextCell(numberOfRingPoints + 1);
      mitk::Vector3D ringPointer;
      for (unsigned int segment = 0; segment < numberOfRingPoints; ++segment)
      {
        ringPointer[0] = 0;
        ringPointer[1] = std::cos((double)(segment) / (double)numberOfRingPoints * 2.0 * vtkMath::Pi());
        ringPointer[2] = std::sin((double)(segment) / (double)numberOfRingPoints * 2.0 * vtkMath::Pi());

        ringPoints->InsertPoint(segment, (ringPointer * halflength).GetDataPointer());

        ringLines->InsertCellPoint(segment);
      }
      ringLines->InsertCellPoint(0);

      // transform ring points (copied from vtkConeSource)
      vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
      t->Translate(center.GetDataPointer());
      double vMag = vtkMath::Norm(axis.GetDataPointer());
      if (axis[0] < 0.0)
      {
        // flip x -> -x to avoid instability
        t->RotateWXYZ(180.0, (axis[0] - vMag) / 2.0, axis[1] / 2.0, axis[2] / 2.0);
        t->RotateWXYZ(180.0, 0, 1, 0);
      }
      else
      {
        t->RotateWXYZ(180.0, (axis[0] + vMag) / 2.0, axis[1] / 2.0, axis[2] / 2.0);
      }

      double thisPoint[3];
      for (unsigned int i = 0; i < numberOfRingPoints; ++i)
      {
        ringPoints->GetPoint(i, thisPoint);
        t->TransformPoint(thisPoint, thisPoint);
        ringPoints->SetPoint(i, thisPoint);
      }

      ringSkeleton->SetPoints(ringPoints);
      ringSkeleton->SetLines(ringLines);

      ringSource = vtkSmartPointer<vtkTubeFilter>::New();
      ringSource->SetInputData(ringSkeleton);
      ringSource->SetNumberOfSides(tubeSides);
      ringSource->SetVaryRadiusToVaryRadiusOff();
      ringSource->SetRadius(shaftRadius);
      ringSource->Update();
      AssignScalarValueTo(ringSource->GetOutput(), vertexValueRing);
    }

    // assemble axis and ring
    vtkSmartPointer<vtkAppendPolyData> appenderGlobal = vtkSmartPointer<vtkAppendPolyData>::New();
    appenderGlobal->AddInputData(axisSource->GetOutput());
    if (drawRing)
    {
      appenderGlobal->AddInputData(ringSource->GetOutput());
    }
    appenderGlobal->Update();

    // make everything shiny by adding normals
    vtkSmartPointer<vtkPolyDataNormals> normalsSource = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalsSource->SetInputConnection(appenderGlobal->GetOutputPort());
    normalsSource->ComputePointNormalsOn();
    normalsSource->ComputeCellNormalsOff();
    normalsSource->SplittingOn();
    normalsSource->Update();

    vtkSmartPointer<vtkPolyData> result = normalsSource->GetOutput();
    return result;
  }

} // unnamed namespace

double mitk::Gizmo::GetLongestRadius() const
{
  double longestAxis = std::max(m_Radius[0], m_Radius[1]);
  longestAxis = std::max(longestAxis, m_Radius[2]);
  return longestAxis;
}

vtkSmartPointer<vtkPolyData> mitk::Gizmo::BuildGizmo()
{
  double longestAxis = GetLongestRadius();

  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();
  appender->AddInputData(BuildAxis(m_Center,
                                   m_AxisX,
                                   longestAxis,
                                   m_AllowRotation,
                                   m_AllowTranslation ? MoveAlongAxisX : NoHandle,
                                   m_AllowRotation ? RotateAroundAxisX : NoHandle,
                                   m_AllowScaling ? ScaleX : NoHandle));
  appender->AddInputData(BuildAxis(m_Center,
                                   m_AxisY,
                                   longestAxis,
                                   m_AllowRotation,
                                   m_AllowTranslation ? MoveAlongAxisY : NoHandle,
                                   m_AllowRotation ? RotateAroundAxisY : NoHandle,
                                   m_AllowScaling ? ScaleY : NoHandle));
  appender->AddInputData(BuildAxis(m_Center,
                                   m_AxisZ,
                                   longestAxis,
                                   m_AllowRotation,
                                   m_AllowTranslation ? MoveAlongAxisZ : NoHandle,
                                   m_AllowRotation ? RotateAroundAxisZ : NoHandle,
                                   m_AllowScaling ? ScaleZ : NoHandle));

  auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(m_Center[0], m_Center[1], m_Center[2]);
  sphereSource->SetRadius(longestAxis * 0.06);
  sphereSource->Update();
  AssignScalarValueTo(sphereSource->GetOutput(), MoveFreely);

  appender->AddInputData(sphereSource->GetOutput());

  appender->Update();
  return appender->GetOutput();
}

void mitk::Gizmo::FollowGeometry(BaseGeometry *geom)
{
  auto observer = itk::SimpleMemberCommand<Gizmo>::New();
  observer->SetCallbackFunction(this, &Gizmo::OnFollowedGeometryModified);

  if (m_FollowedGeometry.IsNotNull())
  {
    m_FollowedGeometry->RemoveObserver(m_FollowerTag);
  }

  m_FollowedGeometry = geom;
  m_FollowerTag = m_FollowedGeometry->AddObserver(itk::ModifiedEvent(), observer);

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

  for (int dim = 0; dim < 3; ++dim)
  {
    m_Radius[dim] = 0.5 * m_FollowedGeometry->GetExtentInMM(dim);
  }

  UpdateRepresentation();
}

mitk::Gizmo::HandleType mitk::Gizmo::GetHandleFromPointDataValue(double value)
{
#define CheckHandleType(type)                                                                                          \
  if (static_cast<int>(value) == static_cast<int>(type))                                                               \
    return type;

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  CheckHandleType(ScaleX);
  CheckHandleType(ScaleY);
  CheckHandleType(ScaleZ);
  return NoHandle;
#undef CheckHandleType
}

mitk::Gizmo::HandleType mitk::Gizmo::GetHandleFromPointID(vtkIdType id)
{
  assert(GetVtkPolyData());
  assert(GetVtkPolyData()->GetPointData());
  assert(GetVtkPolyData()->GetPointData()->GetScalars());
  double dataValue = GetVtkPolyData()->GetPointData()->GetScalars()->GetTuple1(id);
  return GetHandleFromPointDataValue(dataValue);
}

std::string mitk::Gizmo::HandleTypeToString(HandleType type)
{
#define CheckHandleType(candidateType)                                                                                 \
  if (type == candidateType)                                                                                           \
    return std::string(#candidateType);

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  CheckHandleType(ScaleX);
  CheckHandleType(ScaleY);
  CheckHandleType(ScaleZ);
  CheckHandleType(NoHandle);
  return "InvalidHandleType";
#undef CheckHandleType
}
