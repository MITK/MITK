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

#include "mitkRenderWindowViewDirectionController.h"
#include "mitkRenderWindowLayerUtilities.h"

// mitk
#include <mitkRenderingManager.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>

mitk::RenderWindowViewDirectionController::RenderWindowViewDirectionController()
  : m_DataStorage(nullptr)
{
  SetControlledRenderer(RenderingManager::GetInstance()->GetAllRegisteredRenderWindows());
}

mitk::RenderWindowViewDirectionController::~RenderWindowViewDirectionController()
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

void mitk::RenderWindowViewDirectionController::SetControlledRenderer(const RenderingManager::RenderWindowVector &renderWindows)
{
  BaseRenderer* baseRenderer = nullptr;
  for (const auto &renderWindow : renderWindows)
  {
    baseRenderer = BaseRenderer::GetInstance(renderWindow);
    if (nullptr != baseRenderer)
    {
      m_ControlledRenderer.push_back(baseRenderer);
    }
  }
}

void mitk::RenderWindowViewDirectionController::SetControlledRenderer(RendererVector controlledRenderer)
{
  if (m_ControlledRenderer != controlledRenderer)
  {
    // set the new set of controlled renderer
    m_ControlledRenderer = controlledRenderer;
  }
}

void mitk::RenderWindowViewDirectionController::SetViewDirectionOfRenderer(const std::string &viewDirection, BaseRenderer* renderer /*=nullptr*/)
{
  if (nullptr == renderer)
  {

    // set visibility of data node in all controlled renderer
    for (auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
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
      sliceNavigationController->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
    }
    else if ("coronal" == viewDirection)
    {
      sliceNavigationController->SetDefaultViewDirection(mitk::SliceNavigationController::Frontal);
    }
    else if ("sagittal" == viewDirection)
    {
      sliceNavigationController->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
    }

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

  // get all nodes that have a fixed layer for the given renderer
  mitk::NodePredicateProperty::Pointer fixedLayer = mitk::NodePredicateProperty::New("fixedLayer", mitk::BoolProperty::New(true), renderer);

  // combine node predicates
  mitk::NodePredicateAnd::Pointer combinedNodePredicate = mitk::NodePredicateAnd::New(notIncludeInBoundingBox, notAHelperObject, fixedLayer);
  mitk::DataStorage::SetOfObjects::ConstPointer filteredDataNodes = m_DataStorage->GetSubset(combinedNodePredicate);

  // calculate bounding geometry of these nodes
  mitk::TimeGeometry::Pointer bounds = m_DataStorage->ComputeBoundingGeometry3D(filteredDataNodes, "visible", renderer);

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeView(renderer->GetRenderWindow(), bounds);
}