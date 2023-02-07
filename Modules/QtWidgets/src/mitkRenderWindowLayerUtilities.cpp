/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// render window manager module
#include "mitkRenderWindowLayerUtilities.h"

// mitk core
#include <mitkNodePredicateProperty.h>

mitk::RenderWindowLayerUtilities::LayerStack mitk::RenderWindowLayerUtilities::GetLayerStack(const DataStorage* dataStorage, const BaseRenderer* renderer)
{
  LayerStack stackedLayers;
  if (nullptr == dataStorage)
  {
    // no nodes to stack
    return stackedLayers;
  }

  int layer = -1;

  auto allDataNodes = dataStorage->GetAll();
  for (DataStorage::SetOfObjects::ConstIterator it = allDataNodes->Begin(); it != allDataNodes->End(); ++it)
  {
    DataNode::Pointer dataNode = it->Value();
    if (dataNode.IsNull())
    {
      continue;
    }

    bool layerFound = dataNode->GetIntProperty("layer", layer, renderer);
    if (layerFound)
    {
      stackedLayers.insert(std::make_pair(layer, dataNode));
    }
  }
  return stackedLayers;
}

void mitk::RenderWindowLayerUtilities::SetRenderWindowProperties(mitk::DataNode* dataNode, const BaseRenderer* renderer)
{
  // use visibility of existing renderer or common renderer
  // common renderer is used if renderer-specific property does not exist
  bool visible = false;
  bool visibilityProperty = dataNode->GetVisibility(visible, renderer);
  if (true == visibilityProperty)
  {
    // found a visibility property
    dataNode->SetVisibility(visible, renderer);
  }

  // use layer of existing renderer or common renderer
  // common renderer is used if renderer-specific property does not exist
  int layer = -1;
  bool layerProperty = dataNode->GetIntProperty("layer", layer, renderer);
  if (true == layerProperty)
  {
    // found a layer property
    dataNode->SetIntProperty("layer", layer, renderer);
  }
}

void mitk::RenderWindowLayerUtilities::DeleteRenderWindowProperties(mitk::DataNode* dataNode, const BaseRenderer* renderer)
{
  if (nullptr == renderer)
  {
    MITK_ERROR << "Cannot remove general properties. Please provide a specific base renderer.";
    return;
  }

  try
  {
    dataNode->RemoveProperty("visible", renderer->GetName());
  }
  catch (mitk::Exception& e)
  {
    MITK_DEBUG << "Exception caught: " << e.GetDescription() << " Nothing to remove.";
  }

  try
  {
    dataNode->RemoveProperty("layer", renderer->GetName());
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription() << " Nothing to remove.";
  }
}

void mitk::RenderWindowLayerUtilities::TransferRenderWindowProperties(DataNode* dataNode, const BaseRenderer* newRenderer, const BaseRenderer* oldRenderer)
{
  if (nullptr == newRenderer)
  {
    MITK_ERROR << "Cannot transfer properties. Please provide a specific base renderer for the new renderer-specific properties.";
    return;
  }

  // use visibility of existing renderer or common renderer
  // common renderer is used if renderer-specific property does not exist
  bool visible = false;
  bool visibilityProperty = dataNode->GetVisibility(visible, oldRenderer);
  if (true == visibilityProperty)
  {
    // found a visibility property
    dataNode->SetVisibility(visible, newRenderer);
  }

  // use layer of existing renderer or common renderer
  // common renderer is used if renderer-specific property does not exist
  int layer = -1;
  bool layerProperty = dataNode->GetIntProperty("layer", layer, oldRenderer);
  if (true == layerProperty)
  {
    // found a layer property
    dataNode->SetIntProperty("layer", layer, newRenderer);
  }
}
