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

#ifndef MITKRENDERWINDOWLAYERUTILITIES_H
#define MITKRENDERWINDOWLAYERUTILITIES_H

// render window manager module
#include "MitkRenderWindowManagerExports.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

/**
* @brief Render window layer helper functions to retrieve the currently valid layer stack
*/
namespace RenderWindowLayerUtilities
{
  typedef std::map<int, mitk::DataNode::Pointer, std::greater<int>> LayerStack;

  /**
  * The base data node of a renderer is supposed to be on layer 0 (zero), which should be the lowest layer in a render window.
  */
  const int BASE_LAYER_INDEX = 0;
  /**
  * The top layer index, denoting that no valid (positive) layer index is given and therefore the index should be resolved into the topmost layer index.
  */
  const int TOP_LAYER_INDEX = -1;

  /**
  * @brief Return the stack of layers of the given renderer as std::map<int, DataNode::Pointer>, which guarantees ordering of the layers.
  *        Stacked layers are only included if they have their "fixedLayer" property set to true and their "layer" property set.
  *
  *        If "renderer"      = nullptr: a layer stack won't be created and an empty "LayerStack" will be returned.
  *        If "withBaseNode"  = true: include the base node in the layer stack, if existing.
  *        If "withBaseNode"  = false: exclude the base node from the layer stack.
  *
  * @param dataStorage    Pointer to a data storage instance whose data nodes should be checked and possibly be included.
  * @param renderer       Pointer to the renderer instance for which the layer stack should be generated.
  * @param withBaseNode   Boolean to decide whether the base node should be included in or excluded from the layer stack.
  */
  MITKRENDERWINDOWMANAGER_EXPORT LayerStack GetLayerStack(const mitk::DataStorage* dataStorage, const mitk::BaseRenderer* renderer, bool withBaseNode);

} // namespace RenderWindowLayerUtilities

#endif // MITKRENDERWINDOWLAYERUTILITIES_H
