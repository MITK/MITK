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

#include "mitkGetDataStorage.h"
#include "mitkGetSimulationDataNode.h"
#include <mitkNodePredicateDataType.h>
#include <mitkSimulation.h>

mitk::DataNode::Pointer mitk::GetSimulationDataNode(sofa::core::objectmodel::BaseNode::SPtr rootNode)
{
  if (!rootNode)
    return NULL;

  DataStorage::Pointer dataStorage = GetDataStorage();

  if (dataStorage.IsNull())
    return NULL;

  TNodePredicateDataType<Simulation>::Pointer predicate = TNodePredicateDataType<Simulation>::New();
  DataStorage::SetOfObjects::ConstPointer subset = dataStorage->GetSubset(predicate);

  for (DataStorage::SetOfObjects::ConstIterator it = subset->Begin(); it != subset->End(); ++it)
  {
    DataNode::Pointer dataNode = it.Value();
    Simulation::Pointer simulation = static_cast<Simulation*>(dataNode->GetData());

    if (simulation->GetRootNode() == rootNode)
      return dataNode;
  }

  return NULL;
}
