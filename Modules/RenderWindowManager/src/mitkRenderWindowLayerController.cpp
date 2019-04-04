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

mitk::RenderWindowLayerController::RenderWindowLayerController()
  : m_DataStorage(nullptr)
{
  // nothing here
}

void mitk::RenderWindowLayerController::SetDataStorage(DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
  }
}

void mitk::RenderWindowLayerController::SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer)
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
      if (nullptr != renderer)
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
      if (nullptr != renderer)
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
  // use visibility of existing renderer or common renderer
  bool visible = false;
  bool visibilityProperty = dataNode->GetVisibility(visible, renderer);
  if (true == visibilityProperty)
  {
    // found a visibility property
    dataNode->SetVisibility(visible, renderer);
  }

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
      // + 1 indicates inserting the node above the topmost layer
      newLayer = stackedLayers.begin()->first + 1;
    }
    else
    {
      MoveNodeToPosition(dataNode, newLayer, renderer);
      return;
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
      if (nullptr != renderer)
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

bool mitk::RenderWindowLayerController::MoveNodeToPosition(DataNode* dataNode, int newLayer, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  if (nullptr == renderer)
  {
    // move data node to position in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (nullptr != renderer)
      {
        MoveNodeToPosition(dataNode, newLayer, renderer);
        // we don't store/need the returned boolean value
        return false;
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
      if (wasFound && currentLayer != newLayer)
      {
        // move the given data node to the specified layer
        dataNode->SetIntProperty("layer", newLayer, renderer);

        int upperBound;
        int lowerBound;
        int step;
        if (currentLayer < newLayer)
        {
          // move node up
          upperBound = newLayer + 1;
          lowerBound = currentLayer + 1;
          step = -1; // move all other nodes one step down
        }
        else
        {
          upperBound = currentLayer;
          lowerBound = newLayer;
          step = 1; // move all other nodes one step up
        }

        // move all other data nodes between the upper and the lower bound
        for (auto& layer : stackedLayers)
        {
          if (layer.second != dataNode && layer.first < upperBound && layer.first >= lowerBound)
          {
            layer.second->SetIntProperty("layer", layer.first + step, renderer);
          }
          // else: current data node is the selected data node or
          // was previously already above the selected data node or
          // was previously already below the new layer position
        }
        dataNode->Modified();
        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
        return true;
      }
      // else: data node has no layer information or is already at the specified position
    }
    // else: do not work with empty layer stack
  }
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeToFront(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  if (nullptr == renderer)
  {
    // move data node to front in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (nullptr != renderer)
      {
        MoveNodeToFront(dataNode, renderer);
        // we don't store/need the returned boolean value
        return false;
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
        dataNode->Modified();
        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
        return true;
      }
      // else: data node has no layer information or is already the topmost layer node
    }
    // else: do not work with empty layer stack
  }
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeToBack(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  if (nullptr == renderer)
  {
    // move data node to back in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (nullptr != renderer)
      {
        MoveNodeToBack(dataNode, renderer);
        // we don't store/need the returned boolean value
        return false;
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
          // else: current data node is the selected data node or
          // was previously already above the selected data node
        }
        dataNode->Modified();
        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
        return true;
      }
      // else: data node has no layer information or is already the lowermost layer node
    }
    // else: do not work with empty layer stack
  }
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeUp(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  if (nullptr == renderer)
  {
    // move data node down in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (nullptr != renderer)
      {
        MoveNodeUp(dataNode, renderer);
        // we don't store/need the returned boolean value
        return false;
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
          dataNode->Modified();
          mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
          return true;
        }
        // else: layer stack does not contain a layer with the 'currentLayer'data node or
        //       layer is already the topmost layer node
      }
      // else: data node has no layer information
    }
    // else: do not work with empty layer stack
  }
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeDown(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  if (nullptr == renderer)
  {
    // move data node up in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (nullptr != renderer)
      {
        MoveNodeDown(dataNode, renderer);
        // we don't store/need the returned boolean value
        return false;
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
            dataNode->Modified();
            mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
            return true;
          }
          // else: data node is already the lowermost layer node
        }
        // else: layer stack does not contain a layer with the 'currentLayer'
      }
      // else: data node has no layer information
    }
    // else: do not work with empty layer stack
  }
  return false;
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
      if (nullptr != renderer)
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
    if (nullptr != renderer)
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
      if (nullptr != renderer)
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
