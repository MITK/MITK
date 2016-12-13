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

#ifndef MITKRENDERWINDOWLAYERCONTROLLER_H
#define MITKRENDERWINDOWLAYERCONTROLLER_H

// render window manager
#include "MitkRenderWindowManagerExports.h"

// mitk
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /*
  * TODO: really layer 0 (zero)?
  * The base data node of a renderer is supposed to be on layer 0 (zero), which should be the lowest layer in a render window.
  *
  * Functions with 'const mitk::BaseRenderer* renderer' have 'nullptr' as their default argument. Using the nullptr
  * these functions operate on all base renderer.
  */
  class MITKRENDERWINDOWMANAGER_EXPORT RenderWindowLayerController
  {
  public:
    typedef std::vector<BaseRenderer::ConstPointer> RendererVector;

    RenderWindowLayerController();
    virtual ~RenderWindowLayerController();
    /**
    * @brief set the data storage on which to work
    */
    void SetDataStorage(DataStorage::Pointer dataStorage);
    DataStorage::Pointer GetDataStorage() { return m_DataStorage; };
    /**
    * @brief set the controlled base renderer by specifying the corresponding render windows
    */
    void SetControlledRenderer(const RenderingManager::RenderWindowVector &renderWindows);
    /**
    * @brief set the controlled base renderer
    */
    void SetControlledRenderer(RendererVector controlledRenderer);
    RendererVector GetControlledRenderer() { return m_ControlledRenderer; };

    // wrapper functions to modify the layer order / visibility of render window data
    /**
    * @brief set the given node as the base node of the given renderer (nullptr = all renderer)
    */
    void SetBaseDataNode(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief insert the given data node at the specified layer for the given renderer (nullptr = all renderer)
    *
    *   'layer = -1' (default) inserts the given data node at the top of the node stack (topmost layer)
    *   new layer nodes have to have their 'fixedLayer' property set to true
    */
    void InsertLayerNode(DataNode* dataNode, int layer = -1, const BaseRenderer* renderer = nullptr);
    /**
    * @brief 
    */
    void RemoveLayerNode(DataNode* dataNode, const BaseRenderer* renderer = nullptr);

    /**
    * @brief set the node in the given renderer as the topmost layer (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    void MoveNodeToFront(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief set the node in the given renderer as the lowermost layer (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    void MoveNodeToBack(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief move the node in the given renderer one layer down (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    void MoveNodeUp(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief move the node in the given renderer one layer up (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    void MoveNodeDown(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief set the visibility of the given data node for the given renderer (nullptr = all renderer)
    */
    void SetVisibilityOfDataNode(bool visiblity, DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief hide the given data node by setting the 'fixedLayer'property and the 'visible' property of this data node for
    *   all controlled renderer to false
    *
    *   setting the 'visible' property of a data node for a certain renderer will overwrite the same property of the common renderer
    */
    void HideDataNodeInAllRenderer(const DataNode* dataNode);
    /**
    * @brief reset the given render window (nullptr = all renderer):
    *   if 'onlyVisibility = true': set all data nodes for the render window to invisible, except for the base node
    *   if 'onlyVisibility = false': remove all data nodes from the render window, except for the base node
    */
    void ResetRenderer(bool onlyVisibility = true, const BaseRenderer* renderer = nullptr);

  private:

    void InsertLayerNodeInternal(DataNode* dataNode, int layer, const BaseRenderer* renderer = nullptr);

    DataStorage::Pointer m_DataStorage;
    RendererVector m_ControlledRenderer;
  };

} // namespace mitk

#endif // MITKRENDERWINDOWLAYERCONTROLLER_H
