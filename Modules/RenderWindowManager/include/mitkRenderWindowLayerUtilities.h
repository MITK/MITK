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

// render window manager
#include "MitkRenderWindowManagerExports.h"

// mitk
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

/**
* @brief Render window layer helper functions to retrieve the currently valid layer stack
*/
namespace RenderWindowLayerUtilities
{
  typedef std::map<int, mitk::DataNode::Pointer, std::greater<int>> LayerStack;


  /**
  * @brief return the stack of layers of the given renderer as std::map<int, DataNode::Pointer>, which guarantees ordering of the layers
  *       stacked layers are only included, if they have set their 'fixedLayer' property to true
  *
  *       if 'renderer" = nullptr: a layer stack won't be created and an empty 'LayerStack' will be returned
  *       if 'withBaseNode' = true: include the base node (layer = 0) in the layer stack, if existing
  *       if 'withBaseNode' = false: exclude the base node (layer = 0) from the layer stack
  */
  MITKRENDERWINDOWMANAGER_EXPORT LayerStack GetLayerStack(const mitk::DataStorage* dataStorage, const mitk::BaseRenderer* renderer, bool withBaseNode);

} // namespace RenderWindowLayerUtilities

#endif // MITKRENDERWINDOWLAYERUTILITIES_H
