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

#include "mitkRenderWindowLayerController.h"
#include "mitkRenderingManager.h"

// mitk
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>

mitk::RenderWindowLayerController::RenderWindowLayerController()
  : m_DataStorage(nullptr)
{
  SetDataStorage(RenderingManager::GetInstance()->GetDataStorage());
  SetControlledRenderer(RenderingManager::GetInstance()->GetAllRegisteredRenderWindows());
}

mitk::RenderWindowLayerController::RenderWindowLayerController(std::shared_ptr<QmitkRenderWindowDataModel> renderWindowDataModel)
  : m_RenderWindowDataModel(std::move(renderWindowDataModel))
  , m_DataStorage(nullptr)
{
  SetDataStorage(RenderingManager::GetInstance()->GetDataStorage());
  SetControlledRenderer(RenderingManager::GetInstance()->GetAllRegisteredRenderWindows());
}

mitk::RenderWindowLayerController::~RenderWindowLayerController()
{
}

void mitk::RenderWindowLayerController::SetDataStorage(DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
  }

  // TODO: add listener?
}

void mitk::RenderWindowLayerController::SetControlledRenderer(const mitk::RenderingManager::RenderWindowVector &renderWindows)
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

void mitk::RenderWindowLayerController::SetControlledRenderer(RendererVector renderWindows)
{
  m_ControlledRenderer = renderWindows;
}

void mitk::RenderWindowLayerController::SetBaseDataNode(DataNode* dataNode, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
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
    LayerStack stackedLayers = GetLayerStack(renderer, true);
    if (!stackedLayers.empty())
    {
      // see if base layer exists
      LayerStack::iterator layerStackIterator = stackedLayers.find(0);
      if (layerStackIterator != stackedLayers.end())
      {
        // remove the current base data node from the current renderer
        layerStackIterator->second->SetBoolProperty("fixedLayer", false, renderer);
        layerStackIterator->second->SetVisibility(false, renderer);
      }
    }

    // 0 indicates the base data node --> set as new background
    dataNode->SetIntProperty("layer", 0, renderer);
    dataNode->SetBoolProperty("fixedLayer", true, renderer);
    dataNode->SetVisibility(true, renderer);
  }
}

void mitk::RenderWindowLayerController::InsertLayerNode(DataNode* dataNode, int layer /*= -1*/, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
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
    if (0 == layer)
    {
      // 0 indicates the base data node --> set as new background (overwrite current base node if needed)
      SetBaseDataNode(dataNode, renderer);
    }
    else
    {
      InsertLayerNodeInternal(dataNode, layer, renderer);
    }

    mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
  }
}

void mitk::RenderWindowLayerController::InsertLayerNodeInternal(DataNode* dataNode, int newLayer, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
  dataNode->SetBoolProperty("fixedLayer", true, renderer);
  dataNode->SetVisibility(true, renderer);

  // get the layer stack without the base node of the current renderer
  LayerStack stackedLayers = GetLayerStack(renderer, false);
  if (stackedLayers.empty())
  {
    // no layer stack for the current renderer
    if (-1 == newLayer)
    {
      // set given layer as first layer above base layer (= 1)
      newLayer = 1;
      // alternatively: no layer stack for the current renderer -> insert as background node
      //SetBaseDataNode(dataNode, renderer);
    }
  }
  else
  {
    if (-1 == newLayer)
    {
      // get the last value (highest int-key -> topmost layer)
      // +1 indicates inserting the node above the topmost layer
      newLayer = stackedLayers.rbegin()->first + 1;
    }
    else
    {
      // see if layer is already taken
      LayerStack::iterator layerStackIterator = stackedLayers.find(newLayer);
      for (; layerStackIterator != stackedLayers.end(); ++layerStackIterator)
      {
        // move data nodes after the new layer one layer up
        layerStackIterator->second->SetIntProperty("layer", layerStackIterator->first + 1, renderer);
      }
    }
  }
  // update data storage (the "data node model")
  dataNode->SetIntProperty("layer", newLayer, renderer);
  // update render window data model (the "view model")
  m_RenderWindowDataModel->InsertLayerNode(dataNode->GetName(), renderer->GetName());
}

void mitk::RenderWindowLayerController::RemoveLayerNode(int rowIndex, DataNode* dataNode, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == renderer)
  {
    // remove data node from all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        // TODO: does not work here: rowIndex is different for different render window node lists
        RemoveLayerNode(rowIndex, dataNode, renderer);
      }
    }
  }
  else
  {
    dataNode->SetBoolProperty("fixedLayer", false, renderer);
    dataNode->SetVisibility(false, renderer);

    m_RenderWindowDataModel->MoveNodeUp(rowIndex, dataNode->GetName(), renderer->GetName());

    mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
  }
}


bool mitk::RenderWindowLayerController::MoveNodeToFront(DataNode* dataNode, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
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
    LayerStack stackedLayers = GetLayerStack(renderer, false);
    if (!stackedLayers.empty())
    {
      // get the last value (highest int-key -> topmost layer)
      int topmostLayer = stackedLayers.rbegin()->first;
      // get the current layer value of the given data node
      int currentLayer;
      bool wasFound = dataNode->GetIntProperty("layer", currentLayer, renderer);
      if (wasFound && currentLayer < topmostLayer)
      {
        // move the current data node to the current topmost layer
        dataNode->SetIntProperty("layer", topmostLayer, renderer);

        for (auto& layer : stackedLayers)
        {
          if (layer.second != dataNode && layer.first > currentLayer)
          {
            // move all other data nodes one layer down, if their layer was above the layer of the current node
            layer.second->SetIntProperty("layer", layer.first - 1, renderer);
            // alternatively: stack all layers below the topmost layer
            // this would fill gaps between the layers of the renderer, every time a node is moved to front
            // TODO: reverse iteration needed! layer.second->SetIntProperty("layer", --topmostLayer, renderer);
            return true;
          }
          // else: current data node has already been moved to the topmost layer
        }
      }
      // else: data node has no layer information or is already the topmost layer node
    }
    // else: do not work with empty layer stack
  }
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeToBack(DataNode* dataNode, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
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
    LayerStack stackedLayers = GetLayerStack(renderer, false);
    if (!stackedLayers.empty())
    {
      // get the first value (lowest int-key)
      // cannot be the layer 0 (base node) as the base node was excluded by the 'GetLayerStack'-function
      int lowermostLayer = stackedLayers.begin()->first;
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
          if (layer.second != dataNode)
          {
            layer.second->SetIntProperty("layer", layer.first + 1, renderer);
            // alternatively: stack all layers on top of the lowermost layer
            // this would fill gaps between the layers of the renderer, every time a node is moved to back
            //layer.second->SetIntProperty("layer", ++lowermostLayer, renderer);
            return true;
          }
          // else: current data node has already been moved to the lowermost layer
        }
      }
      // else: data node has no layer information or is already the lowermost layer node
    }
    // else: do not work with empty layer stack
  }
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeUp(int rowIndex, DataNode* dataNode, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == renderer)
  {
    // move data node down in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        // TODO: does not work here: rowIndex is different for different render window node lists
        MoveNodeUp(rowIndex, dataNode, renderer);
      }
    }
  }
  else
  {
    // get the layer stack without the base node of the current renderer
    LayerStack stackedLayers = GetLayerStack(renderer, false);
    if (!stackedLayers.empty())
    {
      // get the last value (highest int-key -> topmost layer)
      int topmostLayer = stackedLayers.rbegin()->first;
      // get the current layer value of the given data node
      int currentLayer;
      bool wasFound = dataNode->GetIntProperty("layer", currentLayer, renderer);
      if (wasFound && currentLayer < topmostLayer)
      {
        // current node is not on the topmost layer and therefore can be moved one layer up
        // swap the layers of the dataNode and the dataNode on the next higher layer
        LayerStack::iterator layerStackIterator = stackedLayers.find(currentLayer);
        ++layerStackIterator;
        DataNode::Pointer higherDataNode = layerStackIterator->second;
        dataNode->SetIntProperty("layer", layerStackIterator->first, renderer);
        higherDataNode->SetIntProperty("layer", currentLayer, renderer);

        m_RenderWindowDataModel->MoveNodeUp(rowIndex, dataNode->GetName(), renderer->GetName());

        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
        return true;
      }
      // else: data node has no layer information or is already the topmost layer node
    }
    // else: do not work with empty layer stack
  }
  return false;
}

bool mitk::RenderWindowLayerController::MoveNodeDown(int rowIndex, DataNode* dataNode, const mitk::BaseRenderer* renderer /*= nullptr*/)
{
  if (nullptr == renderer)
  {
    // move data node up in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        // TODO: does not work here: rowIndex is different for different render window node lists
        MoveNodeDown(rowIndex, dataNode, renderer);
      }
    }
  }
  else
  {
    // get the layer stack without the base node of the current renderer
    LayerStack stackedLayers = GetLayerStack(renderer, false);
    if (!stackedLayers.empty())
    {
      // get the first value (lowest int-key)
      // cannot be the layer 0 (base node) as the base node was excluded by the 'GetLayerStack'-function
      int lowermostLayer = stackedLayers.begin()->first;
      // get the current layer value of the given data node
      int currentLayer;
      bool wasFound = dataNode->GetIntProperty("layer", currentLayer, renderer);
      if (wasFound && currentLayer > lowermostLayer)
      {
        // current node is not on the lowermost layer and therefore can be moved one layer down
        // swap the layers of the dataNode and the dataNode on the next lower layer
        LayerStack::iterator layerStackIterator = stackedLayers.find(currentLayer);
        --layerStackIterator;
        DataNode::Pointer lowerDataNode = layerStackIterator->second;
        dataNode->SetIntProperty("layer", layerStackIterator->first, renderer);
        lowerDataNode->SetIntProperty("layer", currentLayer, renderer);

        m_RenderWindowDataModel->MoveNodeDown(rowIndex, dataNode->GetName(), renderer->GetName());

        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
        return true;
      }
      // else: data node has no layer information or is already the lowermost layer node
    }
    // else: do not work with empty layer stack
  }
  return false;
}

void mitk::RenderWindowLayerController::SetVisibilityOfDataNode(bool visibility, DataNode* dataNode, const mitk::BaseRenderer* renderer /*=nullptr*/)
{
  if (nullptr == renderer)
  {
    // set visibility of data node in all controlled renderer
    for (const auto& renderer : m_ControlledRenderer)
    {
      if (renderer.IsNotNull())
      {
        //SetVisibilityOfDataNode(visibility, dataNode, renderer);
        dataNode->SetVisibility(visibility, renderer);
      }
    }
  }
  else
  {
    dataNode->SetVisibility(visibility, renderer);
  }
}

void mitk::RenderWindowLayerController::HideDataNodeInAllRenderer(const DataNode* dataNode)
{
  for (const auto& renderer : m_ControlledRenderer)
  {
    if (renderer.IsNotNull())
    {
      dataNode->GetPropertyList(renderer)->SetBoolProperty("fixedLayer", false);
      dataNode->GetPropertyList(renderer)->SetBoolProperty("visible", false);
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::RenderWindowLayerController::ResetRenderer(bool onlyVisibility /*= true*/, const mitk::BaseRenderer* renderer /*= nullptr*/)
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
    LayerStack stackedLayers = GetLayerStack(renderer, true);
    if (!stackedLayers.empty())
    {
      for (const auto& layer : stackedLayers)
      {
        int layerLevel;
        layer.second->GetIntProperty("layer", layerLevel, renderer);
        if (0 == layerLevel)
        {
          // set base data node visibility to true (layer = 0)
          layer.second->SetVisibility(true, renderer);
        }
        else
        {
          // modify layer node
          if (onlyVisibility)
          {
            // set layer node visibility to false
            layer.second->SetVisibility(false, renderer);
          }
          else
          {

            // remove layer node from current renderer
            layer.second->SetBoolProperty("fixedLayer", false, renderer);
            layer.second->SetVisibility(false, renderer);
          }
        }
      }
    }
  }
}

mitk::RenderWindowLayerController::LayerStack mitk::RenderWindowLayerController::GetLayerStack(const mitk::BaseRenderer* renderer, bool withBaseNode)
{
  LayerStack stackedLayers;
  if (m_DataStorage.IsNull() || renderer == nullptr)
  {
    // no nodes to stack or no renderer selected
    return stackedLayers;
  }

  // see 'QmitkStdMultiWidgetEditor::CreateQtPartControl' for id of GetQmitkRenderWindow
  //m_Renderer = this->GetRenderWindowPart()->GetQmitkRenderWindow("axial" /*"sagittal"*/ /*coronal*/)->GetRenderer();
  int layer = -1;
  mitk::NodePredicateProperty::Pointer helperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
  mitk::NodePredicateNot::Pointer notAHelperObject = mitk::NodePredicateNot::New(helperObject);
  mitk::DataStorage::SetOfObjects::ConstPointer filteredDataNodes = m_DataStorage->GetSubset(notAHelperObject);

  for (DataStorage::SetOfObjects::ConstIterator it = filteredDataNodes->Begin(); it != filteredDataNodes->End(); ++it)
  {
    DataNode::Pointer dataNode = it->Value();
    if (dataNode.IsNull())
    {
      continue;
    }
    
    bool fixedLayer = false;
    dataNode->GetBoolProperty("fixedLayer", fixedLayer, renderer);
    bool layerFound = dataNode->GetIntProperty("layer", layer, renderer);
    if (fixedLayer && layerFound)
    {
      if (layer != 0 || withBaseNode)
      {
        // data node is not on the base layer or the base layer should be included anyway
        stackedLayers.insert(std::make_pair(layer, dataNode));
      }
    }
  }
  return stackedLayers;
}
