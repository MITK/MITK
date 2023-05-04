/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCrosshairManager.h"

#include <mitkException.h>
#include <mitkResliceMethodProperty.h>

#include <mitkCrosshairVtkMapper2D.h>

mitk::CrosshairManager::CrosshairManager(BaseRenderer* baseRenderer)
{
  m_CrosshairDataNode = mitk::DataNode::New();

  std::string rendererName = std::string(baseRenderer->GetName());

  m_CrosshairDataNode->SetProperty("reslice.thickslices", mitk::ResliceMethodProperty::New());
  m_CrosshairDataNode->SetProperty("reslice.thickslices.num", mitk::IntProperty::New(5));
  m_CrosshairDataNode->SetProperty("Crosshair.Gap Size", mitk::IntProperty::New(32));

  // set the crosshair only visible for this specific renderer
  m_CrosshairDataNode->SetVisibility(false);
  m_CrosshairDataNode->SetVisibility(true, baseRenderer);
  m_CrosshairDataNode->SetProperty("name", mitk::StringProperty::New(std::string(rendererName + "crosshairData")));
  m_CrosshairDataNode->SetProperty("helper object", mitk::BoolProperty::New(true));

  m_CrosshairData = CrosshairData::New();
  m_CrosshairDataNode->SetData(m_CrosshairData);
  m_CrosshairDataNode->SetMapper(mitk::BaseRenderer::Standard2D, mitk::CrosshairVtkMapper2D::New());
}

mitk::CrosshairManager::~CrosshairManager()
{
}

void mitk::CrosshairManager::SetCrosshairPosition(const Point3D& selectedPoint)
{
  if (m_CrosshairData.IsNull())
  {
    mitkThrow() << "No crosshair data available. Crosshair is in an invalid state.";
  }

  m_CrosshairData->SetPosition(selectedPoint);
}

void mitk::CrosshairManager::UpdateCrosshairPosition(const SliceNavigationController* sliceNavigationController)
{
  if (m_CrosshairData.IsNull())
  {
    mitkThrow() << "No crosshair data available. Crosshair is in an invalid state.";
  }

  Point3D crosshairPosition = m_CrosshairData->GetPosition();

  auto viewDirection = sliceNavigationController->GetViewDirection();
  unsigned int slicePosition = sliceNavigationController->GetSlice()->GetPos();
  switch (viewDirection)
  {
    case AnatomicalPlane::Original:
      return;
    case AnatomicalPlane::Axial:
    {
      crosshairPosition[2] = slicePosition;
      break;
    }
    case AnatomicalPlane::Coronal:
    {
      crosshairPosition[1] = slicePosition;
      break;
    }
    case AnatomicalPlane::Sagittal:
    {
      crosshairPosition[0] = slicePosition;
      break;
    }
  }

  m_CrosshairData->SetPosition(crosshairPosition);
}

mitk::Point3D mitk::CrosshairManager::GetCrosshairPosition() const
{
  if (m_CrosshairData.IsNull())
  {
    mitkThrow() << "No crosshair data available. Crosshair is in an invalid state.";
  }

  return m_CrosshairData->GetPosition();
}

void mitk::CrosshairManager::SetCrosshairVisibility(bool visible, const BaseRenderer* baseRenderer)
{
  if (m_CrosshairDataNode.IsNull())
  {
    mitkThrow() << "No crosshair data node available. Crosshair is in an invalid state.";
  }

  m_CrosshairDataNode->SetVisibility(visible, baseRenderer);
}

bool mitk::CrosshairManager::GetCrosshairVisibility(const BaseRenderer* baseRenderer) const
{
  if (m_CrosshairDataNode.IsNull())
  {
    mitkThrow() << "No crosshair data node available. Crosshair is in an invalid state.";
  }

  bool visibility = false;
  if (m_CrosshairDataNode->GetVisibility(visibility, baseRenderer))
  {
    return false;
  }

  return visibility;
}

void mitk::CrosshairManager::SetCrosshairGap(unsigned int gapSize)
{
  if (m_CrosshairDataNode.IsNull())
  {
    mitkThrow() << "No crosshair data node available. Crosshair is in an invalid state.";
  }

  m_CrosshairDataNode->SetIntProperty("Crosshair.Gap Size", gapSize);
}

void mitk::CrosshairManager::AddCrosshairNodeToDataStorage(DataStorage* dataStorage)
{
  if (m_CrosshairDataNode.IsNull())
  {
    mitkThrow() << "No crosshair data node available. Crosshair is in an invalid state.";
  }

  if (nullptr == dataStorage)
  {
    mitkThrow() << "Datastorage is invalid. Cannot add crosshair node.";
  }

  if (!dataStorage->Exists(m_CrosshairDataNode))
  {
    dataStorage->Add(m_CrosshairDataNode);
  }
}

void mitk::CrosshairManager::RemoveCrosshairNodeFromDataStorage(DataStorage* dataStorage)
{
  if (m_CrosshairDataNode.IsNull())
  {
    mitkThrow() << "No crosshair data node available. Crosshair is in an invalid state.";
  }

  if (nullptr == dataStorage)
  {
    mitkThrow() << "Datastorage is invalid. Cannot remove crosshair node.";
  }

  dataStorage->Remove(m_CrosshairDataNode);
}
