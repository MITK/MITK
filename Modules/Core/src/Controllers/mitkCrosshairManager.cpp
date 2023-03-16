/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCrosshairManager.h"

#include <mitkLine.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkRenderingManager.h>
#include <mitkResliceMethodProperty.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkSliceNavigationHelper.h>

mitk::CrosshairManager::CrosshairManager(DataStorage* dataStorage, BaseRenderer* baseRenderer)
  : m_DataStorage(dataStorage)
  , m_BaseRenderer(baseRenderer)
  , m_InputTimeGeometry()
  , m_AxialTimeGeometry()
  , m_CoronalTimeGeometry()
  , m_SagittalTimeGeometry()
{
  m_AxialPlaneNode = mitk::DataNode::New();
  m_CoronalPlaneNode = mitk::DataNode::New();
  m_SagittalPlaneNode = mitk::DataNode::New();

  std::string rendererName = std::string(m_BaseRenderer->GetName());
  this->InitializePlaneProperties(m_AxialPlaneNode, std::string(rendererName + "axial.plane"));
  this->InitializePlaneProperties(m_CoronalPlaneNode, std::string(rendererName + "coronal.plane"));
  this->InitializePlaneProperties(m_SagittalPlaneNode, std::string(rendererName + "sagittal.plane"));

  m_ParentNodeForGeometryPlanes = mitk::DataNode::New();
  m_ParentNodeForGeometryPlanes->SetProperty("name", mitk::StringProperty::New(rendererName));
  m_ParentNodeForGeometryPlanes->SetProperty("helper object", mitk::BoolProperty::New(true));
}

mitk::CrosshairManager::~CrosshairManager()
{
  this->RemovePlanesFromDataStorage();
}

void mitk::CrosshairManager::ComputeOrientedTimeGeometries(const TimeGeometry* geometry)
{
  if (nullptr != geometry)
  {
    if (geometry->GetBoundingBoxInWorld()->GetDiagonalLength2() < eps)
    {
      itkWarningMacro("setting an empty bounding-box");
      geometry = nullptr;
    }
  }

  if (m_InputTimeGeometry == geometry)
  {
    return;
  }

  m_InputTimeGeometry = geometry;

  if (m_InputTimeGeometry.IsNull())
  {
    return;
  }

  if (0 == m_InputTimeGeometry->CountTimeSteps())
  {
    return;
  }

  try
  {
    m_AxialTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
      m_InputTimeGeometry, AnatomicalPlane::Axial, false, false, true);
    m_CoronalTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
      m_InputTimeGeometry, AnatomicalPlane::Coronal, false, true, false);
    m_SagittalTimeGeometry = SliceNavigationHelper::CreateOrientedTimeGeometry(
      m_InputTimeGeometry, AnatomicalPlane::Sagittal, true, true, false);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Unable to create oriented time geometries\n"
               << "Reason: " << e.GetDescription();
  }

  this->InitializePlaneData(m_AxialPlaneNode, m_AxialTimeGeometry, m_AxialSlice);
  this->InitializePlaneData(m_CoronalPlaneNode, m_CoronalTimeGeometry, m_CoronalSlice);
  this->InitializePlaneData(m_SagittalPlaneNode, m_SagittalTimeGeometry, m_SagittalSlice);

  RenderingManager::GetInstance()->RequestUpdate(m_BaseRenderer->GetRenderWindow());
}

void mitk::CrosshairManager::SetCrosshairPosition(const Point3D& selectedPoint)
{
  if (m_AxialPlaneNode.IsNull() || m_CoronalPlaneNode.IsNull() || m_SagittalPlaneNode.IsNull())
  {
    return;
  }

  if (m_AxialTimeGeometry.IsNull() || m_CoronalTimeGeometry.IsNull() || m_SagittalTimeGeometry.IsNull())
  {
    return;
  }

  this->SetCrosshairPosition(selectedPoint, m_AxialPlaneNode, m_AxialTimeGeometry, m_AxialSlice);
  this->SetCrosshairPosition(selectedPoint, m_CoronalPlaneNode, m_CoronalTimeGeometry, m_CoronalSlice);
  this->SetCrosshairPosition(selectedPoint, m_SagittalPlaneNode, m_SagittalTimeGeometry, m_SagittalSlice);
}

mitk::Point3D mitk::CrosshairManager::GetCrosshairPosition() const
{
  if (m_InputTimeGeometry.IsNull())
  {
    // return null-point since we don't show the crosshair anyway
    return Point3D(0.0);
  }

  Point3D point = m_InputTimeGeometry->GetCenterInWorld();

  PlaneGeometry* axialPlaneGeometry = nullptr;
  PlaneGeometry* coronalPlaneGeometry = nullptr;
  PlaneGeometry* sagittalPlaneGeometry = nullptr;

  // get the currently selected time point
  auto selectedTimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  try
  {
    axialPlaneGeometry =
      SliceNavigationHelper::GetCurrentPlaneGeometry(m_AxialTimeGeometry, selectedTimePoint, m_AxialSlice);
    coronalPlaneGeometry =
      SliceNavigationHelper::GetCurrentPlaneGeometry(m_CoronalTimeGeometry, selectedTimePoint, m_CoronalSlice);
    sagittalPlaneGeometry =
      SliceNavigationHelper::GetCurrentPlaneGeometry(m_SagittalTimeGeometry, selectedTimePoint, m_SagittalSlice);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Unable to get plane geometries. Using default center point.\n"
               << "Reason: " << e.GetDescription();
  }

  Line3D line;
  if (nullptr != axialPlaneGeometry && nullptr != coronalPlaneGeometry &&
      axialPlaneGeometry->IntersectionLine(coronalPlaneGeometry, line))
  {
    if (nullptr != sagittalPlaneGeometry && sagittalPlaneGeometry->IntersectionPoint(line, point))
    {
      return point;
    }
  }

  // return input geometry center point if no intersection point was found
  return point;
}

void mitk::CrosshairManager::UpdateSlice(const SliceNavigationController* sliceNavigationController)
{
  auto viewDirection = sliceNavigationController->GetViewDirection();
  unsigned int slicePosition = sliceNavigationController->GetSlice()->GetPos();
  switch (viewDirection)
  {
    case AnatomicalPlane::Original:
      return;
    case AnatomicalPlane::Axial:
    {
      m_AxialSlice = slicePosition;
      this->UpdatePlaneSlice(m_AxialPlaneNode, m_AxialTimeGeometry, m_AxialSlice);
      break;
    }
    case AnatomicalPlane::Coronal:
    {
      m_CoronalSlice = slicePosition;
      this->UpdatePlaneSlice(m_CoronalPlaneNode, m_CoronalTimeGeometry, m_CoronalSlice);
      break;
    }
    case AnatomicalPlane::Sagittal:
    {
      m_SagittalSlice = slicePosition;
      this->UpdatePlaneSlice(m_SagittalPlaneNode, m_SagittalTimeGeometry, m_SagittalSlice);
      break;
    }
  }
}

void mitk::CrosshairManager::SetCrosshairVisibility(bool visible)
{
  if (m_AxialPlaneNode.IsNull() || m_CoronalPlaneNode.IsNull() || m_SagittalPlaneNode.IsNull())
  {
    return;
  }
  
  m_AxialPlaneNode->SetVisibility(visible, m_BaseRenderer);
  m_CoronalPlaneNode->SetVisibility(visible, m_BaseRenderer);
  m_SagittalPlaneNode->SetVisibility(visible, m_BaseRenderer);
}

bool mitk::CrosshairManager::GetCrosshairVisibility() const
{
  if (m_AxialPlaneNode.IsNull() || m_CoronalPlaneNode.IsNull() || m_SagittalPlaneNode.IsNull())
  {
    return false;
  }

  bool axialVisibility = false;
  if (m_AxialPlaneNode->GetVisibility(axialVisibility, m_BaseRenderer))
  {
    bool coronalVisibility = false;
    if (m_CoronalPlaneNode->GetVisibility(coronalVisibility, m_BaseRenderer))
    {
      bool sagittalVisibility = false;
      if (m_SagittalPlaneNode->GetVisibility(sagittalVisibility, m_BaseRenderer))
      {
        if (axialVisibility == coronalVisibility && coronalVisibility == sagittalVisibility)
        {
          return axialVisibility;
        }
      }
    }
  }

  mitkThrow() << "Invalid state of plane visibility.";
}

void mitk::CrosshairManager::SetCrosshairGap(unsigned int gapSize)
{
  m_AxialPlaneNode->SetIntProperty("Crosshair.Gap Size", gapSize);
  m_CoronalPlaneNode->SetIntProperty("Crosshair.Gap Size", gapSize);
  m_SagittalPlaneNode->SetIntProperty("Crosshair.Gap Size", gapSize);
}

void mitk::CrosshairManager::AddPlanesToDataStorage()
{
  if (nullptr == m_DataStorage)
  {
    return;
  }

  if (m_AxialPlaneNode.IsNull() || m_CoronalPlaneNode.IsNull()
    || m_SagittalPlaneNode.IsNull() || m_ParentNodeForGeometryPlanes.IsNull())
  {
    return;
  }

  this->AddPlaneToDataStorage(m_ParentNodeForGeometryPlanes, nullptr);
  this->AddPlaneToDataStorage(m_AxialPlaneNode, m_ParentNodeForGeometryPlanes);
  this->AddPlaneToDataStorage(m_CoronalPlaneNode, m_ParentNodeForGeometryPlanes);
  this->AddPlaneToDataStorage(m_SagittalPlaneNode, m_ParentNodeForGeometryPlanes);
}

void mitk::CrosshairManager::RemovePlanesFromDataStorage()
{
  if (nullptr == m_DataStorage)
  {
    return;
  }

  if (m_AxialPlaneNode.IsNotNull() && m_CoronalPlaneNode.IsNotNull()
      && m_SagittalPlaneNode.IsNotNull() && m_ParentNodeForGeometryPlanes.IsNotNull())
  {
    m_DataStorage->Remove(m_AxialPlaneNode);
    m_DataStorage->Remove(m_CoronalPlaneNode);
    m_DataStorage->Remove(m_SagittalPlaneNode);
    m_DataStorage->Remove(m_ParentNodeForGeometryPlanes);
  }
}

void mitk::CrosshairManager::InitializePlaneProperties(DataNode::Pointer planeNode, const std::string& planeName)
{
  planeNode->SetProperty("reslice.thickslices", mitk::ResliceMethodProperty::New());
  planeNode->SetProperty("reslice.thickslices.num", mitk::IntProperty::New(5));
  planeNode->SetProperty("Crosshair.Gap Size", mitk::IntProperty::New(32));

  // set the crosshair only visible for this specific renderer
  planeNode->SetVisibility(false);
  planeNode->SetVisibility(true, m_BaseRenderer);
  planeNode->SetProperty("name", mitk::StringProperty::New(planeName));
  planeNode->SetProperty("helper object", mitk::BoolProperty::New(true));
}

void mitk::CrosshairManager::InitializePlaneData(DataNode::Pointer planeNode, const TimeGeometry* timeGeometry, unsigned int& slice)
{
  if (planeNode.IsNull())
  {
    return;
  }

  if (nullptr == timeGeometry)
  {
    return;
  }

  // get the BaseGeometry of the selected time point
  auto selectedTimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  auto currentGeometry = timeGeometry->GetGeometryForTimePoint(selectedTimePoint);
  if (nullptr == currentGeometry)
  {
    // time point not valid for the time geometry
    mitkThrow() << "Cannot extract a base geometry. A time point is selected that is not covered by"
                << "the given time geometry. Selected time point: " << selectedTimePoint;
  }

  const auto* slicedGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(currentGeometry.GetPointer());
  if (nullptr == slicedGeometry)
  {
    return;
  }

  slice = slicedGeometry->GetSlices() / 2; // center slice
  PlaneGeometryData::Pointer planeData = PlaneGeometryData::New();
  planeData->SetPlaneGeometry(slicedGeometry->GetPlaneGeometry(slice));
  planeNode->SetData(planeData);
  planeNode->SetMapper(mitk::BaseRenderer::Standard2D, mitk::PlaneGeometryDataMapper2D::New());
}

void mitk::CrosshairManager::UpdatePlaneSlice(DataNode::Pointer planeNode, const TimeGeometry* timeGeometry, unsigned int slice)
{
  mitk::PlaneGeometry* planeGeometry = nullptr;
  // get the currently selected time point
  auto selectedTimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  try
  {
    planeGeometry = SliceNavigationHelper::GetCurrentPlaneGeometry(timeGeometry, selectedTimePoint, slice);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Unable to get plane geometries\n"
      << "Reason: " << e.GetDescription();
  }

  if (nullptr == planeGeometry)
  {
    return;
  }

  PlaneGeometryData::Pointer planeData = dynamic_cast<PlaneGeometryData*>(planeNode->GetData());
  if (nullptr == planeData)
  {
    return;
  }

  planeData->SetPlaneGeometry(planeGeometry);
  planeNode->SetData(planeData);
}

void mitk::CrosshairManager::SetCrosshairPosition(const Point3D& selectedPoint,
                                                  DataNode::Pointer planeNode,
                                                  const TimeGeometry* timeGeometry,
                                                  unsigned int& slice)
{
  int selectedSlice = -1;
  try
  {
    selectedSlice = SliceNavigationHelper::SelectSliceByPoint(timeGeometry, selectedPoint);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Unable to select a slice by the given point " << selectedPoint << "\n"
               << "Reason: " << e.GetDescription();
  }

  if (-1 == selectedSlice)
  {
    return;
  }

  slice = selectedSlice;
  mitk::PlaneGeometry* planeGeometry = nullptr;
  // get the currently selected time point
  auto selectedTimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  try
  {
    planeGeometry = SliceNavigationHelper::GetCurrentPlaneGeometry(timeGeometry, selectedTimePoint, slice);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Unable to get plane geometries\n"
      << "Reason: " << e.GetDescription();
  }

  if (nullptr == planeGeometry)
  {
    return;
  }

  PlaneGeometryData::Pointer planeData = dynamic_cast<PlaneGeometryData*>(planeNode->GetData());
  if (nullptr == planeData)
  {
    return;
  }

  planeData->SetPlaneGeometry(planeGeometry);
  planeNode->SetData(planeData);
}

void mitk::CrosshairManager::AddPlaneToDataStorage(DataNode::Pointer planeNode, DataNode::Pointer parent)
{
  if (!m_DataStorage->Exists(planeNode)
    && (nullptr == parent || m_DataStorage->Exists(parent)))
  {
    try
    {
      m_DataStorage->Add(planeNode, parent);
    }
    catch (std::invalid_argument& /*e*/)
    {
      return;
    }
  }
}
