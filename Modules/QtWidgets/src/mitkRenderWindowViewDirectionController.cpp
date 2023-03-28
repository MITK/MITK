/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderWindowViewDirectionController.h"

// mitk core
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>

mitk::RenderWindowViewDirectionController::RenderWindowViewDirectionController()
  : m_DataStorage(nullptr)
{
  // nothing here
}

void mitk::RenderWindowViewDirectionController::SetDataStorage(DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
  }
}

void mitk::RenderWindowViewDirectionController::SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  if (m_ControlledRenderer != controlledRenderer)
  {
    // set the new set of controlled renderer
    m_ControlledRenderer = controlledRenderer;
  }
}

void mitk::RenderWindowViewDirectionController::SetViewDirectionOfRenderer(const std::string& viewDirection, BaseRenderer* renderer/* =nullptr*/)
{
  if (nullptr == renderer)
  {
    // set visibility of data node in all controlled renderer
    for (auto& renderer : m_ControlledRenderer)
    {
      if (nullptr != renderer)
      {
        SetViewDirectionOfRenderer(viewDirection, renderer);
      }
    }
  }
  else
  {
    mitk::SliceNavigationController* sliceNavigationController = renderer->GetSliceNavigationController();
    if ("axial" == viewDirection)
    {
      sliceNavigationController->SetDefaultViewDirection(AnatomicalPlane::Axial);
    }
    else if ("coronal" == viewDirection)
    {
      sliceNavigationController->SetDefaultViewDirection(AnatomicalPlane::Coronal);
    }
    else if ("sagittal" == viewDirection)
    {
      sliceNavigationController->SetDefaultViewDirection(AnatomicalPlane::Sagittal);
    }

    if ("3D" == viewDirection)
    {
      renderer->SetMapperID(mitk::BaseRenderer::Standard3D);
    }
    else
    {
      renderer->SetMapperID(mitk::BaseRenderer::Standard2D);

    }
    // initialize the views to the bounding geometry
    InitializeViewByBoundingObjects(renderer);
  }
}

void mitk::RenderWindowViewDirectionController::SetViewDirectionOfRenderer(AnatomicalPlane viewDirection , BaseRenderer* renderer/* =nullptr*/)
{
  if (nullptr == renderer)
  {
    // set visibility of data node in all controlled renderer
    for (auto& renderer : m_ControlledRenderer)
    {
      if (nullptr != renderer)
      {
        SetViewDirectionOfRenderer(viewDirection, renderer);
      }
    }
  }
  else
  {
    mitk::SliceNavigationController* sliceNavigationController = renderer->GetSliceNavigationController();
    sliceNavigationController->SetDefaultViewDirection(viewDirection);

    // initialize the views to the bounding geometry
    InitializeViewByBoundingObjects(renderer);
  }
}

void mitk::RenderWindowViewDirectionController::InitializeViewByBoundingObjects(const BaseRenderer* renderer)
{
  if (nullptr == m_DataStorage || nullptr == renderer)
  {
    return;
  }

  // get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateProperty::Pointer includeInBoundingBox = mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false));
  mitk::NodePredicateNot::Pointer notIncludeInBoundingBox = mitk::NodePredicateNot::New(includeInBoundingBox);

  // get all non-helper objects
  mitk::NodePredicateProperty::Pointer helperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
  mitk::NodePredicateNot::Pointer notAHelperObject = mitk::NodePredicateNot::New(helperObject);

  // combine node predicates
  mitk::NodePredicateAnd::Pointer combinedNodePredicate = mitk::NodePredicateAnd::New(notIncludeInBoundingBox, notAHelperObject);
  mitk::DataStorage::SetOfObjects::ConstPointer filteredDataNodes = m_DataStorage->GetSubset(combinedNodePredicate);

  // calculate bounding geometry of these nodes
  auto bounds = m_DataStorage->ComputeBoundingGeometry3D(filteredDataNodes, "visible", renderer);

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeView(renderer->GetRenderWindow(), bounds);
}
