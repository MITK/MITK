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

// render window manager module
#include "MitkRenderWindowManagerExports.h"
#include "mitkRenderWindowLayerUtilities.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /**
  *
  * Functions with 'const mitk::BaseRenderer* renderer' have 'nullptr' as their default argument. Using the nullptr
  * these functions operate on all base renderer.
  */
  class MITKRENDERWINDOWMANAGER_EXPORT RenderWindowLayerController
  {
  public:
    typedef std::vector<BaseRenderer::ConstPointer> RendererVector;

    RenderWindowLayerController();
    /**
    * @brief Set the data storage on which to work.
    */
    void SetDataStorage(DataStorage::Pointer dataStorage);
    DataStorage::Pointer GetDataStorage() { return m_DataStorage; };
    /**
    * @brief Set the controlled base renderer by specifying the corresponding render windows.
    */
    void SetControlledRenderer(const RenderingManager::RenderWindowVector &renderWindows);
    /**
    * @brief Set the controlled base renderer.
    */
    void SetControlledRenderer(RendererVector controlledRenderer);
    RendererVector GetControlledRenderer() { return m_ControlledRenderer; };

    // wrapper functions to modify the layer order / visibility of render window data
    /**
    * @brief Set the given node as the base node of the given renderer.
    *
    * @param dataNode   The data node whose layer is to be modified.
    * @param renderer   Pointer to the renderer instance for which the data node property should be modified.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    void SetBaseDataNode(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Insert the given data node at the specified layer for the given renderer.
    *
    * @param dataNode   The data node that should be inserted.
    * @param layer      The layer value for the "layer" property of the data node (insertion level).
                        "layer = RenderWindowLayerUtilities::TOP_LAYER_INDEX" (default) inserts the given data node at the top of the node stack (topmost layer).
    * @param renderer   Pointer to the renderer instance for which the data node should be inserted.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    *
    * @post After a successful call, the "fixedLayer" and "visibility" property will be true and the "layer" property will be set correctly.
    */
    void InsertLayerNode(DataNode* dataNode, int layer = RenderWindowLayerUtilities::TOP_LAYER_INDEX, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Remove the given data node for the given renderer.
    *
    * @param dataNode   The data node that should be removed.
    * @param renderer   Pointer to the renderer instance for which the data node should be removed.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    *
    * @post After a successful call, the "fixedLayer" and "visibility" property will be false and the "layer" property will be deleted.
    */
    void RemoveLayerNode(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Set the node in the given renderer as the topmost layer. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    void MoveNodeToFront(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Set the node in the given renderer as the lowermost layer. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    void MoveNodeToBack(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Move the node in the given renderer one layer down. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    void MoveNodeUp(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Move the node in the given renderer one layer up. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    void MoveNodeDown(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Set the visibility of the given data node for the given renderer.
    *
    * @param visibility Boolean to set the "visible" property of the given data node.
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be modified.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    *
    * @post After a successful call , the "visibility" property will be set to the "visibility" value.
    */
    void SetVisibilityOfDataNode(bool visiblity, DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Hide the given data node by setting the "visible" property of the data node for
    *        all controlled renderer to false.
    *        Later setting the "visible" property of the data node for a certain renderer will overwrite
    *        the same property of the common renderer.
    *
    * @param dataNode   The data node that should be hid.
    *
    * @post After a successful call , the "visibility" property will be set to the false.
    */
    void HideDataNodeInAllRenderer(const DataNode* dataNode);
    /**
    * @brief Reset the given render window:
    *        If "onlyVisibility = true": set all data nodes for the given render window to invisible, except for the base node.
    *        If "onlyVisibility = false": remove all data nodes from the render window, except for the base node.
    *
    * @param visibility Boolean to define the reset mode.
    * @param renderer   Pointer to the renderer instance for which the data node should be reset.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    *
    * @post After a successful call , the "visibility" property will be set to the "false" value (except for the base node).
    *       If "onlyVisibility = false": additionally the "fixedLayer" property will be false and the "layer" property will be deleted.
    */
    void ResetRenderer(bool onlyVisibility = true, const BaseRenderer* renderer = nullptr);

  private:

    void InsertLayerNodeInternal(DataNode* dataNode, int layer, const BaseRenderer* renderer = nullptr);

    DataStorage::Pointer m_DataStorage;
    RendererVector m_ControlledRenderer;
  };

} // namespace mitk

#endif // MITKRENDERWINDOWLAYERCONTROLLER_H
