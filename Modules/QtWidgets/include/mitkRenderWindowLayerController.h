/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindowLayerController_h
#define mitkRenderWindowLayerController_h

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "mitkRenderWindowLayerUtilities.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /**
  * The RenderWindowLayerController is used to manipulate the 'layer' and 'visible' property of a given data node.
  * The 'layer' property is used to denote the layer level of a data node. Data from nodes on higher layer level are rendered
  * on top of data from nodes on lower layer level. It can be changed using the 'MoveNode*'-functions.
  *
  * To view the data of a data node only in a specific renderer, the "InsertLayerNode'-function should be used. It inserts the
  * given node into the specified renderer and sets the corresponding properties.
  * Inserting and showing a data node in a specific renderer / render window, will overwrite the properties of the common renderer view.
  *
  * For more information about the data node properties for specific renderer, see mitk::DataNode- and mitk::PropertyList-classes.
  *
  * Functions with 'mitk::BaseRenderer* renderer' have 'nullptr' as their default argument. Using the nullptr
  * these functions operate on the general property list.
  * Giving a specific base renderer will modify the property list of the given renderer.
  */
  class MITKQTWIDGETS_EXPORT RenderWindowLayerController
  {
  public:

    RenderWindowLayerController();
    /**
    * @brief Set the data storage on which to work.
    */
    void SetDataStorage(DataStorage::Pointer dataStorage);

    // wrapper functions to modify the layer order / visibility of render window data
    /**
    * @brief Insert the given data node at the specified layer for the given renderer.
    *
    * @param dataNode   The data node that should be inserted.
    * @param layer      The layer value for the "layer" property of the data node (insertion level).
                        "layer = RenderWindowLayerUtilities::TOP_LAYER_INDEX" (default) inserts the given data node at the top of the node stack (topmost layer).
    * @param renderer   Pointer to the renderer instance for which the data node should be inserted.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    *
    * @post After a successful call, the "visibility" property will be true and the "layer" property will be set correctly.
    */
    void InsertLayerNode(DataNode* dataNode, int layer = RenderWindowLayerUtilities::TOP_LAYER_INDEX, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Move the data node to the given layer. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param layer      The layer to which the data node should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected and the
    *                   common property list will be modified.
    */
    bool MoveNodeToPosition(DataNode* dataNode, int layer, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Set the node in the given renderer as the topmost layer. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    bool MoveNodeToFront(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Set the node in the given renderer as the lowermost layer. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    bool MoveNodeToBack(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Move the node in the given renderer one layer down. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    bool MoveNodeUp(DataNode* dataNode, const BaseRenderer* renderer = nullptr);
    /**
    * @brief Move the node in the given renderer one layer up. This will change only the "layer" property.
    *
    * @param dataNode   The data node that should be moved.
    * @param renderer   Pointer to the renderer instance for which the data node should be moved.
    *                   If it is a nullptr (default) all controlled renderer will be affected.
    */
    bool MoveNodeDown(DataNode* dataNode, const BaseRenderer* renderer = nullptr);

  private:

    DataStorage::Pointer m_DataStorage;
  };

} // namespace mitk

#endif
