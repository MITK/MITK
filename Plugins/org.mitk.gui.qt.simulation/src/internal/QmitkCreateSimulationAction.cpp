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

#include "QmitkCreateSimulationAction.h"
#include <mitkIOUtil.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIRenderingManager.h>
#include <mitkSimulationTemplate.h>
#include <algorithm>

static std::string CreateFileName(mitk::DataNode::Pointer dataNode)
{
  std::string path;
  dataNode->GetStringProperty("path", path);

  std::string name;
  dataNode->GetStringProperty("name", name);

  if (name.length() < 5)
  {
    name += ".scn";
  }
  else
  {
    std::string ext = name.substr(name.length() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

    if (ext != ".scn" && ext != ".xml")
      name += ".scn";
  }

  return path + "/" + name;
}

static void InitializeViews(mitk::DataStorage::Pointer dataStorage)
{
  mitk::NodePredicateNot::Pointer predicate = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer subset = dataStorage->GetSubset(predicate);
  mitk::TimeSlicedGeometry::Pointer geometry = dataStorage->ComputeBoundingGeometry3D(subset);

  mitk::RenderingManager::GetInstance()->InitializeViews(geometry);
}

QmitkCreateSimulationAction::QmitkCreateSimulationAction()
{
}

QmitkCreateSimulationAction::~QmitkCreateSimulationAction()
{
}

void QmitkCreateSimulationAction::Run(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  mitk::DataNode::Pointer dataNode;

  foreach(dataNode, selectedNodes)
  {
    if (dynamic_cast<mitk::SimulationTemplate*>(dataNode->GetData()) != NULL)
    {
      mitk::SimulationTemplate* simulationTemplate = static_cast<mitk::SimulationTemplate*>(dataNode->GetData());
      std::string contents = simulationTemplate->CreateSimulation();

      if (contents.empty())
      {
        MITK_ERROR << "Could not create simulation from template '" << dataNode->GetName() << "'!";
        continue;
      }

      std::string fileName = CreateFileName(dataNode);

      std::ofstream file(fileName.c_str());
      file << contents;
      file.close();

      std::vector<std::string> fileNames;
      fileNames.push_back(fileName);

      mitk::DataNode::Pointer simulationNode = mitk::IOUtil::LoadDataNode(fileName);

      if (simulationNode.IsNotNull())
      {
        m_DataStorage->Add(simulationNode, dataNode);
        InitializeViews(m_DataStorage);
      }
    }
  }
}

void QmitkCreateSimulationAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreateSimulationAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkCreateSimulationAction::SetDecimated(bool)
{
}

void QmitkCreateSimulationAction::SetSmoothed(bool)
{
}
