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

#include "mitkSceneIOTestScenarioProvider.h"

// --------------- SceneIOTestScenarioProvider::Scenario ---------------

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::Scenario::BuildDataStorage() const
{
  return (*m_ScenarioProvider.*m_ProviderMethod)(); // calls function
}

mitk::SceneIOTestScenarioProvider::Scenario::Scenario(const std::string& _key,
    const SceneIOTestScenarioProvider* _scenarioProvider,
    SceneIOTestScenarioProvider::BuilderMethodPointer _providerMethod,
    bool _isSerializable,
    const std::string& _referenceArchiveFilename,
    bool _isReferenceLoadable)
: key(_key)
, serializable(_isSerializable)
, referenceArchiveFilename(_referenceArchiveFilename)
, referenceArchiveLoadable(_isReferenceLoadable)
, m_ScenarioProvider(_scenarioProvider)
, m_ProviderMethod(_providerMethod)
{
}

// --------------- SceneIOTestScenarioProvider ---------------

mitk::SceneIOTestScenarioProvider::ScenarioList mitk::SceneIOTestScenarioProvider::GetAllScenarios() const
{
  return m_Scenarios;
}


void mitk::SceneIOTestScenarioProvider::AddScenario(const std::string& key,
                                                    BuilderMethodPointer creator,
                                                    bool isSerializable,
                                                    const std::string& referenceArchiveFilename,
                                                    bool isReferenceLoadable)
{
  Scenario newScenario(key, this, creator, isSerializable, referenceArchiveFilename, isReferenceLoadable);
  m_Scenarios.push_back(newScenario);
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::EmptyStorage() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();
  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::OneEmptyNode() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  mitk::DataNode::Pointer node = DataNode::New();
  storage->Add(node);

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::OneEmptyNamedNode() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  mitk::DataNode::Pointer node = DataNode::New();
  node->SetName("Urmel");
  storage->Add(node);

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::ManyTopLevelNodes() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  for ( auto i = 0; i < m_HowMuchIsMany; ++i ) {
    mitk::DataNode::Pointer node = DataNode::New();
    std::stringstream s;
    s << "Node #" << i;
    node->SetName(s.str());
    storage->Add(node);
  }

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::LineOfManyOnlyChildren() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  mitk::DataNode::Pointer parent;
  for ( auto i = 0; i < m_HowMuchIsMany; ++i ) {
    mitk::DataNode::Pointer node = DataNode::New();
    std::stringstream s;
    s << "Node #" << i;
    node->SetName(s.str());
    storage->Add(node, parent);
    parent = node;
  }

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::ComplicatedFamilySituation() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();
  return storage;
}

