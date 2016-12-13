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

#include "mitkRenderWindowLayerUtilities.h"

// mitk
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>

RenderWindowLayerUtilities::LayerStack RenderWindowLayerUtilities::GetLayerStack(const mitk::DataStorage* dataStorage, const mitk::BaseRenderer* renderer, bool withBaseNode)
{
  LayerStack stackedLayers;
  if (nullptr == dataStorage || nullptr == renderer)
  {
    // no nodes to stack or no renderer selected
    return stackedLayers;
  }

  int layer = -1;
  mitk::NodePredicateProperty::Pointer helperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
  mitk::NodePredicateNot::Pointer notAHelperObject = mitk::NodePredicateNot::New(helperObject);

  mitk::NodePredicateProperty::Pointer fixedLayer = mitk::NodePredicateProperty::New("fixedLayer", mitk::BoolProperty::New(true), renderer);

  // combine node predicates
  mitk::NodePredicateAnd::Pointer combinedNodePredicate = mitk::NodePredicateAnd::New(notAHelperObject, fixedLayer);
  mitk::DataStorage::SetOfObjects::ConstPointer filteredDataNodes = dataStorage->GetSubset(combinedNodePredicate);

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = filteredDataNodes->Begin(); it != filteredDataNodes->End(); ++it)
  {
    mitk::DataNode::Pointer dataNode = it->Value();
    if (dataNode.IsNull())
    {
      continue;
    }

    bool layerFound = dataNode->GetIntProperty("layer", layer, renderer);
    if (layerFound)
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
