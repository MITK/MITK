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
#include "mitkRenderWindowLayerUtilities.h"

// mitk core
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

mitk::RenderWindowLayerUtilities::LayerStack mitk::RenderWindowLayerUtilities::GetLayerStack(const DataStorage* dataStorage, const BaseRenderer* renderer, bool withBaseNode)
{
  LayerStack stackedLayers;
  if (nullptr == dataStorage || nullptr == renderer)
  {
    // no nodes to stack or no renderer selected
    return stackedLayers;
  }

  int layer = -1;
  NodePredicateAnd::Pointer combinedNodePredicate = GetRenderWindowPredicate(renderer);
  DataStorage::SetOfObjects::ConstPointer filteredDataNodes = dataStorage->GetSubset(combinedNodePredicate);
  for (DataStorage::SetOfObjects::ConstIterator it = filteredDataNodes->Begin(); it != filteredDataNodes->End(); ++it)
  {
    DataNode::Pointer dataNode = it->Value();
    if (dataNode.IsNull())
    {
      continue;
    }

    bool layerFound = dataNode->GetIntProperty("layer", layer, renderer);
    if (layerFound)
    {
      if (BASE_LAYER_INDEX != layer|| withBaseNode)
      {
        // data node is not on the base layer or the base layer should be included anyway
        stackedLayers.insert(std::make_pair(layer, dataNode));
      }
    }
  }
  return stackedLayers;
}

mitk::NodePredicateAnd::Pointer mitk::RenderWindowLayerUtilities::GetRenderWindowPredicate(const BaseRenderer* renderer)
{
  NodePredicateAnd::Pointer renderWindowPredicate = NodePredicateAnd::New();

  NodePredicateProperty::Pointer helperObject = NodePredicateProperty::New("helper object", BoolProperty::New(true));
  NodePredicateProperty::Pointer fixedLayer = NodePredicateProperty::New("fixedLayer", BoolProperty::New(true), renderer);

  renderWindowPredicate->AddPredicate(NodePredicateNot::New(helperObject));
  renderWindowPredicate->AddPredicate(fixedLayer);

  return renderWindowPredicate;
}
