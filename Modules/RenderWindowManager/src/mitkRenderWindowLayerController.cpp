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

// render window manager module
#include "mitkRenderWindowLayerController.h"

// mitk core
#include "mitkRenderingManager.h"

mitk::RenderWindowLayerController::RenderWindowLayerController()
  : m_DataStorage(nullptr)
{
  SetControlledRenderer(RenderingManager::GetInstance()->GetAllRegisteredRenderWindows());
}

void mitk::RenderWindowLayerController::SetDataStorage(DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
  }
}

void mitk::RenderWindowLayerController::SetControlledRenderer(const RenderingManager::RenderWindowVector &renderWindows)
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

void mitk::RenderWindowLayerController::SetControlledRenderer(RendererVector controlledRenderer)
{
  if (m_ControlledRenderer != controlledRenderer)
  {
    // set the new set of controlled renderer
    m_ControlledRenderer = controlledRenderer;
  }
}

void mitk::RenderWindowLayerController::SetBaseDataNode(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // set the data node as base data node in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        SetBaseDataNode(dataNode, renderer);
      }
    }
  }
  else
  {
    // get the layer stack with the base data node of the current renderer
    RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer, true);
    if (!stackedLayers.empty())
    {
      // see if base layer exists
      RenderWindowLayerUtilities::LayerStack::iterator layerStackIterator = stackedLayers.find(RenderWindowLayerUtilities::BASE_LAYER_INDEX);
      if (layerStackIterator != stackedLayers.end())
      {
        // remove the current base data node from the current renderer
        layerStackIterator->second->GetPropertyList(renderer)->DeleteProperty("layer");
        layerStackIterator->second->SetBoolProperty("fixedLayer", false, renderer);
        layerStackIterator->second->SetVisibility(false, renderer);
      }
    }

    // "RenderWindowLayerUtilities::BASE_LAYER_INDEX" indicates the base data node --> set as new background
    dataNode->SetIntProperty("layer", RenderWindowLayerUtilities::BASE_LAYER_INDEX, renderer);
    dataNode->SetBoolProperty("fixedLayer", true, renderer);
    dataNode->SetVisibility(true, renderer);
    dataNode->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
  }
}

void mitk::RenderWindowLayerController::InsertLayerNode(DataNode* dataNode, int layer /*= RenderWindowLayerUtilities::TOP_LAYER_INDEX*/, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // insert data node in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        InsertLayerNode(dataNode, layer, renderer);
      }
    }
  }
  else
  {
    if (RenderWindowLayerUtilities::BASE_LAYER_INDEX == layer)
    {
      // "RenderWindowLayerUtilities::BASE_LAYER_INDEX" indicates the base data node --> set as new background (overwrite current base node if needed)
      SetBaseDataNode(dataNode, renderer);
    }
    else
    {
      InsertLayerNodeInternal(dataNode, layer, renderer);
    }
  }
}

void mitk::RenderWindowLayerController::InsertLayerNodeInternal(DataNode* dataNode, int newLayer, const BaseRenderer* renderer /*= nullptr*/)
{
  dataNode->SetBoolProperty("fixedLayer", true, renderer);
  dataNode->SetVisibility(true, renderer);

  // get the layer stack without the base node of the current renderer
  RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer, false);
  if (stackedLayers.empty())
  {
    // no layer stack for the current renderer
    if (RenderWindowLayerUtilities::TOP_LAYER_INDEX == newLayer)
    {
      // set given layer as first layer above base layer (= 1)
      newLayer = 1;
      // alternatively: no layer stack for the current renderer -> insert as background node
      //SetBaseDataNode(dataNode, renderer);
    }
  }
  else
  {
    if (RenderWindowLayerUtilities::TOP_LAYER_INDEX == newLayer)
    {
      // get the first value (highest int-key -> topmost layer)
      // +1 indicates inserting the node above the topmost layer
      newLayer = stackedLayers.begin()->first + 1;
    }
    else
    {
      // see if layer is already taken
      RenderWindowLayerUtilities::LayerStack::iterator layerStackIterator = stackedLayers.find(newLayer);
      for (; layerStackIterator != stackedLayers.end(); ++layerStackIterator)
      {
        // move data nodes after the new layer one layer up
        layerStackIterator->second->SetIntProperty("layer", layerStackIterator->first + 1, renderer);
      }
    }
  }
  // update data storage (the "data node model")
  dataNode->SetIntProperty("layer", newLayer, renderer);
  dataNode->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
}

void mitk::RenderWindowLayerController::RemoveLayerNode(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // remove data node from all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        RemoveLayerNode(dataNode, renderer);
      }
    }
  }
  else
  {
    // "remove" node from the renderer list
    dataNode->GetPropertyList(renderer)->DeleteProperty("layer");
    dataNode->SetBoolProperty("fixedLayer", false, renderer);
    dataNode->SetVisibility(false, renderer);
    dataNode->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
  }
}

void mitk::RenderWindowLayerController::MoveNodeToFront(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // move data node to front in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        MoveNodeToFront(dataNode, renderer);
      }
    }
  }
  else
  {
    // get the layer stack without the base node of the current renderer
    RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer, false);
    if (!stackedLayers.empty())
    {
      // get the first value (highest int-key -> topmost layer)
      int topmostLayer = stackedLayers.begin()->first;
      // get the current layer value of the given data node
      int currentLayer;
      bool wasFound = dataNode->GetIntProperty("layer", currentLayer, renderer);
      if (wasFound && currentLayer < topmostLayer)
      {
        // move the current data node above the current topmost layer
        dataNode->SetIntProperty("layer", topmostLayer+1, renderer);
      }
      // else: data node has no layer information or is already the topmost layer node
    }
    // else: do not work with empty layer stack
  }
}

void mitk::RenderWindowLayerController::MoveNodeToBack(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // move data node to back in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        MoveNodeToBack(dataNode, renderer);
      }
    }
  }
  else
  {
    // get the layer stack without the base node of the current renderer
    RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer, false);
    if (!stackedLayers.empty())
    {
      // get the last value (lowest int-key)
      // cannot be the base layer as the base node was excluded by the 'GetLayerStack'-function
      int lowermostLayer = stackedLayers.rbegin()->first;
      // get the current layer value of the given data node
      int currentLayer;
      bool wasFound = dataNode->GetIntProperty("layer", currentLayer, renderer);
      if (wasFound && currentLayer > lowermostLayer)
      {
        // move the current data node to the current lowermost layer
        dataNode->SetIntProperty("layer", lowermostLayer, renderer);
        // move all other data nodes one layer up
        for (auto& layer : stackedLayers)
        {
          if (layer.second != dataNode && layer.first < currentLayer)
          {
            layer.second->SetIntProperty("layer", layer.first + 1, renderer);
          }
          // else: current data node has already been moved to the lowermost layer
        }
      }
      // else: data node has no layer information or is already the lowermost layer node
    }
    // else: do not work with empty layer stack
  }
}

void mitk::RenderWindowLayerController::MoveNodeUp(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // move data node down in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        MoveNodeUp(dataNode, renderer);
      }
    }
  }
  else
  {
    // get the layer stack without the base node of the current renderer
    RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer, false);
    if (!stackedLayers.empty())
    {
      // get the current layer value of the given data node
      int currentLayer;
      bool wasFound = dataNode->GetIntProperty("layer", currentLayer, renderer);
      if (wasFound)
      {
        // get the current layer in the map of stacked layers
        RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIterator = stackedLayers.find(currentLayer);
        if (layerStackIterator != stackedLayers.end() && layerStackIterator != stackedLayers.begin())
        {
          // found the element in the map, at different position than 'begin' ->
          // current node is not on the topmost layer and therefore can be moved one layer up
          // swap the layers of the dataNode and the dataNode on the next higher layer (previous map element)
          RenderWindowLayerUtilities::LayerStack::const_iterator prevLayerStackIterator = std::prev(layerStackIterator);
          dataNode->SetIntProperty("layer", prevLayerStackIterator->first, renderer);
          prevLayerStackIterator->second->SetIntProperty("layer", currentLayer, renderer);
          //prevLayerStackIterator->second->Modified();
          dataNode->Modified();
          mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
        }
        // else: layer stack does not contain a layer with the 'currentLayer'data node or
        //       layer is already the topmost layer node
      }
      // else: data node has no layer information
    }
    // else: do not work with empty layer stack
  }
}

void mitk::RenderWindowLayerController::MoveNodeDown(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // move data node up in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        MoveNodeDown(dataNode, renderer);
      }
    }
  }
  else
  {
    // get the layer stack without the base node of the current renderer
    RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer, false);
    if (!stackedLayers.empty())
    {
      // get the current layer value of the given data node
      int currentLayer;
      bool wasFound = dataNode->GetIntProperty("layer", currentLayer, renderer);
      if (wasFound)
      {
        // get the current layer in the map of stacked layers
        RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIterator = stackedLayers.find(currentLayer);
        if (layerStackIterator != stackedLayers.end())
        {
          // found the element in the map ...
          RenderWindowLayerUtilities::LayerStack::const_iterator nextLayerStackIterator = std::next(layerStackIterator);
          if (nextLayerStackIterator != stackedLayers.end())
          {
            // ... and found a successor ->
            // current node is not on the lowermost layer and therefore can be moved one layer down
            // swap the layers of the dataNode and the dataNode on the next lower layer (next map element)
            dataNode->SetIntProperty("layer", nextLayerStackIterator->first, renderer);
            nextLayerStackIterator->second->SetIntProperty("layer", currentLayer, renderer);
            //nextLayerStackIterator->second->Modified();
            dataNode->Modified();
            mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
          }
          // else: data node is already the lowermost layer node
        }
        // else: layer stack does not contain a layer with the 'currentLayer'
      }
      // else: data node has no layer information
    }
    // else: do not work with empty layer stack
  }
}

void mitk::RenderWindowLayerController::SetVisibilityOfDataNode(bool visibility, DataNode* dataNode, const BaseRenderer* renderer /*=nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  if (nullptr == renderer)
  {
    // set visibility of data node in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        SetVisibilityOfDataNode(visibility, dataNode, renderer);
      }
    }
  }
  else
  {
    dataNode->SetVisibility(visibility, renderer);
    dataNode->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
  }
}

void mitk::RenderWindowLayerController::HideDataNodeInAllRenderer(const DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    return;
  }

  for (const auto& renderer : m_ControlledRenderer)
  {
    if (renderer.IsNotNull())
    {
      dataNode->GetPropertyList(renderer)->SetBoolProperty("visible", false);
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::RenderWindowLayerController::ResetRenderer(bool onlyVisibility /*= true*/, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == renderer)
  {
    // reset all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        ResetRenderer(onlyVisibility, renderer);
      }
    }
  }
  else
  {
    // get the layer stack with the base node of the current renderer
    RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer, true);
    if (!stackedLayers.empty())
    {
      for (const auto& layer : stackedLayers)
      {
        int layerLevel;
        layer.second->GetIntProperty("layer", layerLevel, renderer);
        if (RenderWindowLayerUtilities::BASE_LAYER_INDEX == layerLevel)
        {
          // set base data node visibility to true
          layer.second->SetVisibility(true, renderer);
        }
        else
        {
          // set visibility of all other data nodes to false
          layer.second->SetVisibility(false, renderer);

          // modify layer node
          if (!onlyVisibility)
          {
            // clear mode: additionally remove layer node from current renderer
            layer.second->GetPropertyList(renderer)->DeleteProperty("layer");
            layer.second->SetBoolProperty("fixedLayer", false, renderer);
          }
        }
        layer.second->Modified();
      }

      mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
    }
  }
}
