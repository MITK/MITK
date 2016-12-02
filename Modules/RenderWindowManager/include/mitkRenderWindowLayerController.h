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
#include <QmitkRenderWindowDataModel.h>

// mitk
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /*
  * TODO: generate layer stack on demand or keep layer stack of each render window on hand
  *
  * TODO: really layer 0 (zero)?
  * The base data node of a renderer is supposed to be on layer 0 (zero), which should be the lowest layer in a render window.
  *
  * Functions with 'const mitk::BaseRenderer* renderer' have 'nullptr' as their default argument. Using the nullptr
  * these functions operate on all base renderer.
  */
  class MITKRENDERWINDOWMANAGER_EXPORT RenderWindowLayerController
  {
  public:
    typedef std::vector<mitk::BaseRenderer::ConstPointer> RendererVector;
    typedef std::map<int, DataNode::Pointer> LayerStack;

    RenderWindowLayerController();
    RenderWindowLayerController(std::shared_ptr<QmitkRenderWindowDataModel> renderWindowDataModel);

    virtual ~RenderWindowLayerController();
    /**
    * @brief set the data storage on which to work
    */
    void SetDataStorage(DataStorage::Pointer dataStorage);
    DataStorage::Pointer GetDataStorage() { return m_DataStorage; };
    /**
    * @brief set the controlled base renderer by specifying the corresponding render windows
    */
    void SetControlledRenderer(const mitk::RenderingManager::RenderWindowVector &renderWindows);
    /**
    * @brief set the controlled base renderer
    */
    void SetControlledRenderer(RendererVector renderWindows);
    RendererVector GetControlledRenderer() { return m_ControlledRenderer; };
    //itkGetMacro(ControlledRenderer, RendererVector);

    // wrapper functions to modify the layer order / visibility of render window data
    /**
    * @brief set the given node as the base node of the given renderer (nullptr = all renderer)
    */
    void SetBaseDataNode(DataNode* dataNode, const mitk::BaseRenderer* renderer = nullptr);
    /**
    * @brief insert the given data node at the specified layer for the given renderer (nullptr = all renderer)
    *
    *   'layer = -1' (default) inserts the given data node at the top of the node stack (topmost layer)
    *   new layer nodes have to have their 'fixedLayer' property set to true
    */
    void InsertLayerNode(DataNode* dataNode, int layer = -1, const mitk::BaseRenderer* renderer = nullptr);
    /**
    * @brief 
    */
    void RemoveLayerNode(int rowIndex, DataNode* dataNode, const mitk::BaseRenderer* renderer = nullptr);

    /**
    * @brief set the node in the given renderer as the topmost layer (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    bool MoveNodeToFront(DataNode* dataNode, const mitk::BaseRenderer* renderer = nullptr);
    /**
    * @brief set the node in the given renderer as the lowermost layer (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    bool MoveNodeToBack(DataNode* dataNode, const mitk::BaseRenderer* renderer = nullptr);
    /**
    * @brief move the node in the given renderer one layer down (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    bool MoveNodeUp(int rowIndex, DataNode* dataNode, const mitk::BaseRenderer* renderer = nullptr);
    /**
    * @brief move the node in the given renderer one layer up (nullptr = all renderer)
    *
    * @return true, if the node was successfully moved, false otherwise
    */
    bool MoveNodeDown(int rowIndex, DataNode* dataNode, const mitk::BaseRenderer* renderer = nullptr);
    /**
    * @brief set the visibility of the given data node for the given renderer (nullptr = all renderer)
    */
    void SetVisibilityOfDataNode(bool visiblity, DataNode* dataNode, const mitk::BaseRenderer* renderer = nullptr);
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
    void ResetRenderer(bool onlyVisibility = true, const mitk::BaseRenderer* renderer = nullptr);
    /**
    * @brief return the stack of layers of the given renderer as std::map<int, DataNode::Pointer>, which guarantees ordering of the layers
    *       stacked layers are only included, if they have set their 'fixedLayer' property to true
    *
    *       if 'renderer" = nullptr: a layer stack won't be created and an empty 'LayerStack' will be returned
    *       if 'withBaseNode' = true: include the base node (layer = 0) in the layer stack, if existing
    *       if 'withBaseNode' = false: exclude the base node (layer = 0) from the layer stack
    */
    LayerStack GetLayerStack(const mitk::BaseRenderer* renderer, bool withBaseNode);

  private:

    void InsertLayerNodeInternal(DataNode* dataNode, int layer, const mitk::BaseRenderer* renderer = nullptr);

    std::shared_ptr<QmitkRenderWindowDataModel> m_RenderWindowDataModel;
    DataStorage::Pointer m_DataStorage;
    RendererVector m_ControlledRenderer;
  };

} // namespace mitk

#endif // MITKRENDERWINDOWLAYERCONTROLLER_H
