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

#include "mitkDataStorageCompare.h"

mitk::DataStorageCompare::DataStorageCompare(const mitk::DataStorage* reference,
                                             const mitk::DataStorage* test,
                                             Tests flags)
: m_TestAspects(flags)
, m_ReferenceDS(reference)
, m_TestDS(test)
, m_HierarchyPassed(false)
, m_DataPassed(false)
, m_PropertiesPassed(false)
, m_MappersPassed(false)
, m_InteractorsPassed(false)
, m_AspectsFailed(0)
{

}

bool mitk::DataStorageCompare::CompareVerbose()
{
  return Compare(true);
}

bool mitk::DataStorageCompare::Compare(bool verbose)
{
  if ( m_TestAspects & CMP_Hierarchy )
  {
    m_HierarchyPassed = CompareHierarchy(verbose);
    if (!m_HierarchyPassed) ++m_AspectsFailed;
  }

  if ( m_TestAspects & CMP_Data )
  {
    m_DataPassed = CompareData(verbose);
    if (!m_DataPassed) ++m_AspectsFailed;
  }

  if ( m_TestAspects & CMP_Properties )
  {
    m_PropertiesPassed = CompareProperties(verbose);
    if (!m_PropertiesPassed) ++m_AspectsFailed;
  }

  if ( m_TestAspects & CMP_Mappers )
  {
    m_MappersPassed = CompareMappers(verbose);
    if (!m_MappersPassed) ++m_AspectsFailed;
  }

  if ( m_TestAspects & CMP_Interactors )
  {
    m_InteractorsPassed = CompareInteractors(verbose);
    if (!m_InteractorsPassed) ++m_AspectsFailed;
  }


  // ...

  if (verbose)
      Report();

  return m_AspectsFailed == 0;
}


void mitk::DataStorageCompare::Report()
{
  MITK_INFO << "Comparison results:";
  MITK_INFO << "  Hierarchy comparison:   " << (m_TestAspects & CMP_Hierarchy ? (m_HierarchyPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Data comparison:        " << (m_TestAspects & CMP_Data ? (m_DataPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Properties comparison:  " << (m_TestAspects & CMP_Properties ? (m_PropertiesPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Mappers comparison:     " << (m_TestAspects & CMP_Mappers ? (m_MappersPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Interactors comparison: " << (m_TestAspects & CMP_Interactors ? (m_InteractorsPassed ? "pass" : "fail") : "skipped");

  if (m_AspectsFailed == 0)
    MITK_INFO << "  Summary:                 ALL PASSED";
  else
    MITK_INFO << "  Summary:                " << m_AspectsFailed << " failures";
}


void mitk::DataStorageCompare::DescribeHierarchyOfNodes(DataStorage::ConstPointer storage, HierarchyDescriptorMap& result)
{
  result.clear();
  if (storage.IsNull()) return;

  mitk::DataStorage::SetOfObjects::ConstPointer allNodes = storage->GetAll();
  for (auto node : *allNodes)
  {
    std::string descriptor = GenerateHierarchyDescriptor(node, storage);
    result.insert( std::make_pair(descriptor, node) );
  }
}

std::string mitk::DataStorageCompare::GenerateNodeDescriptor(mitk::DataNode::Pointer node)
{
  if (node.IsNull()) return "nullptr";

  std::string thisDataDescriptor = "nullptr";
  mitk::BaseData* data = node->GetData();
  if (data != nullptr)
      thisDataDescriptor = data->GetNameOfClass(); //  TODO simplify?

  std::string thisNodeName = node->GetName(); // TODO simplify?

  std::string thisNodesDescriptor = std::string("_") + thisDataDescriptor + "_(" + thisNodeName + ")";
  return thisNodesDescriptor;
}

std::string mitk::DataStorageCompare::GenerateHierarchyDescriptor(mitk::DataNode::Pointer node, mitk::DataStorage::ConstPointer storage)
{
  std::string thisNodesDescriptor = GenerateNodeDescriptor(node);
  mitk::DataStorage::SetOfObjects::ConstPointer parents = storage->GetSources(node, nullptr, true); // direct sources without filter

  if (!parents->empty())
  {
    thisNodesDescriptor += " <(";
    for (auto parent : *parents)
    {
      if (parent != parents->front()) // join by '+'
      {
        thisNodesDescriptor += " + ";
      }
      thisNodesDescriptor += GenerateHierarchyDescriptor(parent, storage);
    }
    thisNodesDescriptor += ")";
  }

  return thisNodesDescriptor;
}

bool mitk::DataStorageCompare::CompareHierarchy(bool verbose)
{
  DescribeHierarchyOfNodes(m_ReferenceDS, m_RefNodesByHierarchy);
  DescribeHierarchyOfNodes(m_TestDS, m_TestNodesByHierarchy);

  int numberOfMisMatches = 0;

  for (auto entry : m_RefNodesByHierarchy)
  {
    const std::string& key = entry.first;
    const mitk::DataNode::Pointer& node = entry.second;

    //if (verbose)
    //  MITK_INFO << "Test node '" << node->GetName() << "'";

    unsigned int timesInReference = m_RefNodesByHierarchy.count(key);
    unsigned int timesInTest = m_TestNodesByHierarchy.count(key);

    if (timesInTest != timesInReference)
    {
      ++numberOfMisMatches;
      if (verbose)
      {
        MITK_WARN << "### Hierarchy mismatch problem";
        MITK_WARN << "  Reference storage has " << timesInReference << " node(s), test storage " << timesInTest;
        MITK_WARN << "  Node name '" << node->GetName() << "'";
        MITK_WARN << "  Reference hierarchy descriptor: " << key;
      }
    }
  }

  // TODO test also keys that are _only_ in test!

  if (verbose && numberOfMisMatches > 0)
  MITK_INFO << "Dumping test storage because there were errors:";
  for (auto entry : m_TestNodesByHierarchy)
  {
    const std::string& key = entry.first;
    const mitk::DataNode::Pointer& node = entry.second;
    MITK_INFO << "  Test node '" << node->GetName() << "', hierarchy : " << key;
  }

  return numberOfMisMatches == 0;
}


bool mitk::DataStorageCompare::CompareData(bool verbose)
{
  return false;
}


bool mitk::DataStorageCompare::CompareProperties(bool verbose)
{
  return false;
}


bool mitk::DataStorageCompare::CompareMappers(bool verbose)
{
  return false;
}


bool mitk::DataStorageCompare::CompareInteractors(bool verbose)
{
  return false;
}

