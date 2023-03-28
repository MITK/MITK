/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

void mitk::RenderWindowLayerController::InsertLayerNode(DataNode* dataNode, int layer /*= RenderWindowLayerUtilities::TOP_LAYER_INDEX*/, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return;
  }

  RenderWindowLayerUtilities::SetRenderWindowProperties(dataNode, renderer);

  // get the layer stack of the current renderer
  RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer);
  if (stackedLayers.empty())
  {
    // no layer stack for the current renderer
    if (RenderWindowLayerUtilities::TOP_LAYER_INDEX == layer)
    {
      // set given layer as first layer
      layer = 0;
    }
  }
  else
  {
    if (RenderWindowLayerUtilities::TOP_LAYER_INDEX == layer)
    {
      // get the first value (highest int-key -> topmost layer)
      // + 1 indicates inserting the node above the topmost layer
      layer = stackedLayers.begin()->first + 1;
    }
    else
    {
      MoveNodeToPosition(dataNode, layer, renderer);
      return;
    }
  }
  // update data storage (the "data node model")
  dataNode->SetIntProperty("layer", layer, renderer);
  dataNode->Modified();
  if (nullptr == renderer)
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  }
  else
  {
    mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
  }
}

bool mitk::RenderWindowLayerController::MoveNodeToPosition(DataNode* dataNode, int newLayer, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  // get the layer stack of the current renderer
  RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer);
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
      if (nullptr == renderer)
      {
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
      else
      {
        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
      }

      return true;
    }
    // else: data node has no layer information or is already at the specified position
  }
  // else: do not work with empty layer stack
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeToFront(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  // get the layer stack of the current renderer
  RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer);
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
      if (nullptr == renderer)
      {
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      }
      else
      {
        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
      }
      return true;
    }
    // else: data node has no layer information or is already the topmost layer node
  }
  // else: do not work with empty layer stack
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeToBack(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  // get the layer stack of the current renderer
  RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer);
  if (!stackedLayers.empty())
  {
    // get the last value (lowest int-key)
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
      if (nullptr == renderer)
      {
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      }
      else
      {
        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
      }
      return true;
    }
    // else: data node has no layer information or is already the lowermost layer node
  }
  // else: do not work with empty layer stack
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeUp(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  // get the layer stack of the current renderer
  RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer);
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
        if (nullptr == renderer)
        {
          mitk::RenderingManager::GetInstance()->RequestUpdateAll();

        }
        else
        {
          mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
        }
        return true;
      }
      // else: layer stack does not contain a layer with the 'currentLayer'data node or
      //       layer is already the topmost layer node
    }
    // else: data node has no layer information
  }
  // else: do not work with empty layer stack
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeDown(DataNode* dataNode, const BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == dataNode)
  {
    return false;
  }

  // get the layer stack of the current renderer
  RenderWindowLayerUtilities::LayerStack stackedLayers = RenderWindowLayerUtilities::GetLayerStack(m_DataStorage, renderer);
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
  return false;
}
