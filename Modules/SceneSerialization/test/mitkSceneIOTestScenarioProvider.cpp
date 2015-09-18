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

#define AddNode(name) \
  storage->Add(name);

#define DefineNode(name) \
  mitk::DataNode::Pointer name = mitk::DataNode::New(); \
  name->SetName( #name );

#define DefNode0(name) \
  DefineNode(name) \
  AddNode(name)

#define DefNode1(source, name) \
  DefineNode(name) \
  storage->Add(name, source);

#define DefNode2(source1, source2, name) \
  DefineNode(name) \
  { mitk::DataStorage::SetOfObjects::Pointer sources = mitk::DataStorage::SetOfObjects::New(); \
  sources->push_back(source1); \
  sources->push_back(source2); \
  storage->Add(name, sources); }

#define DefNode3(source1, source2, source3, name) \
  DefineNode(name) \
  { mitk::DataStorage::SetOfObjects::Pointer sources = mitk::DataStorage::SetOfObjects::New(); \
  sources->push_back(source1); \
  sources->push_back(source2); \
  sources->push_back(source3); \
  storage->Add(name, sources); }



mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::ComplicatedFamilySituation() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  // constructing a hierarchy with multiple levels
  // and a couple of multiple parent relations.

  // Source image : http://4.bp.blogspot.com/_57lEz2uo9yw/TTGWNhjsWPI/AAAAAAAAA5M/yMauv61ULXc/s1600/web-technology-family-tree-large.jpg

  // The red tree

  // two real ones
  DefNode0(Cpp)
  DefNode0(Basic)

  // descendants
  DefNode1(Cpp, Java)
  DefNode1(Cpp, Perl)
  DefNode1(Cpp, CSharp)

  DefNode1(Java, JSP)

  DefNode1(Perl, PHP)
  DefNode1(Perl, Ruby)
  DefNode1(Perl, Python)

  DefNode1(PHP, OOPHP)
  DefNode1(PHP, PHPLibs)

  DefNode1(Ruby, RubyRails);

  DefNode1(Basic, VBNet)

  // interesting ones
  DefNode2(Basic, VBNet, VBScript)
  DefNode2(CSharp, VBNet, ASPNet)
  DefNode2(ASPNet, VBScript, ClassicASP)

  DefNode1(ClassicASP, SHTML_SSI)
  DefNode1(ASPNet, ATLAS)

  // some green items
  DefNode0(ECMAScript)
  DefNode0(XHTML)
  DefNode1(ECMAScript, JavaScript)
  DefNode1(JavaScript, DHTML)

  // finally
  DefNode3(DHTML, XHTML, RubyRails, Ajax)

  return storage;
}

