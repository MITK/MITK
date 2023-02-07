/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindowLayerUtilities_h
#define mitkRenderWindowLayerUtilities_h

// qt widgets module
#include "MitkQtWidgetsExports.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

/**
* @brief Render window layer helper functions to retrieve the currently valid layer stack
*/
namespace mitk
{
  namespace RenderWindowLayerUtilities
  {
    typedef std::vector<BaseRenderer*> RendererVector;
    typedef std::map<int, DataNode::Pointer, std::greater<int>> LayerStack;

    /**
    * The top layer index, denoting that no valid (positive) layer index is given and therefore the index should be resolved into the topmost layer index.
    */
    const int TOP_LAYER_INDEX = -1;

    /**
    * @brief Return the stack of layers of the given renderer as std::map<int, DataNode::Pointer>, which guarantees ordering of the layers.
    *        Stacked layers are only included if they have their "layer" property set.
    *
    *        If "renderer"      = nullptr: a layer stack won't be created and an empty "LayerStack" will be returned.
    *
    * @param dataStorage    Pointer to a data storage instance whose data nodes should be checked and possibly be included.
    * @param renderer       Pointer to the renderer instance for which the layer stack should be generated.
    */
    MITKQTWIDGETS_EXPORT LayerStack GetLayerStack(const DataStorage* dataStorage, const BaseRenderer* renderer);
    /**
    * @brief Set renderer-specific properties to mark a data node as 'managed by the specific renderer'.
    *        In order for a renderer to manage a data node, the 'visible' and the 'layer' property are set and
    *        allow to individually render a set of nodes with a specific renderer.
    *        The last two mentioned properties are set so that they initially have the same value as the corresponding
    *        global property.
    */
    MITKQTWIDGETS_EXPORT void SetRenderWindowProperties(DataNode* dataNode, const BaseRenderer* renderer);

    MITKQTWIDGETS_EXPORT void DeleteRenderWindowProperties(DataNode* dataNode, const BaseRenderer* renderer);

    MITKQTWIDGETS_EXPORT void TransferRenderWindowProperties(DataNode* dataNode, const BaseRenderer* newRenderer, const BaseRenderer* oldRenderer);


  } // namespace RenderWindowLayerUtilities
} // namespace mitk

#endif
