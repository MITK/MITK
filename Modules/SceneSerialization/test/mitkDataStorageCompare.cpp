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
#include "mitkBaseDataEqual.h"

#include "usModuleContext.h"
#include "usGetModuleContext.h"
#include "usLDAPFilter.h"

mitk::DataStorageCompare::DataStorageCompare(const mitk::DataStorage* reference,
                                             const mitk::DataStorage* test,
                                             Tests flags,
                                             double eps)
: m_TestAspects(flags)
, m_ReferenceDS(reference)
, m_TestDS(test)
, m_HierarchyPassed(false)
, m_DataPassed(false)
, m_PropertiesPassed(false)
, m_MappersPassed(false)
, m_InteractorsPassed(false)
, m_AspectsFailed(0)
, m_Eps(eps)
{
  BaseDataEqual::RegisterCoreEquals();
}

bool mitk::DataStorageCompare::CompareVerbose()
{
  return Compare(true);
}

bool mitk::DataStorageCompare::Compare(bool verbose)
{
  DescribeHierarchyOfNodes(m_ReferenceDS, m_RefNodesByHierarchy);
  DescribeHierarchyOfNodes(m_TestDS, m_TestNodesByHierarchy);

  if ( m_TestAspects & CMP_Hierarchy )
  {
    m_HierarchyPassed = CompareHierarchy(verbose);
    if (!m_HierarchyPassed) ++m_AspectsFailed;
  }

  if ( m_TestAspects & CMP_Data )
  {
    m_DataPassed = CompareData(m_Eps, verbose);
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
      thisDataDescriptor = data->GetNameOfClass();

  std::string thisNodeName = node->GetName();

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
  int numberOfMisMatches = 0;

  // check for each reference storage entry
  // if it can be found in test storage with
  // an identical hierarchy descriptor.
  // Compare just counts because there might be
  // multiple nodes that have the same name / type / etc.
  for (auto entry : m_RefNodesByHierarchy)
  {
    const std::string& key = entry.first;
    const mitk::DataNode::Pointer& node = entry.second;

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

  // test also keys that are _only_ in test!
  for (auto entry : m_TestNodesByHierarchy)
  {
    const std::string& key = entry.first;
    const mitk::DataNode::Pointer& node = entry.second;

    unsigned int timesInReference = m_RefNodesByHierarchy.count(key);
    unsigned int timesInTest = m_TestNodesByHierarchy.count(key);

    // we already tested all items in reference storage.
    // Here we want to test additional items in test storage.
    if (timesInTest > timesInReference)
    {
      ++numberOfMisMatches;

      if (verbose)
      {
        MITK_WARN << "### Hierarchy mismatch problem";
        MITK_WARN << "  Test storage has more nodes (" << timesInReference << ") than reference storage (" << timesInTest << ")";
        MITK_WARN << "  Node name '" << node->GetName() << "'";
        MITK_WARN << "  Reference hierarchy descriptor: " << key;
      }
    }
  }

  // for debug purposes we provide a dump of the test storage
  // in error cases. This can be compared to the test case
  // by a programmer.
  if (verbose && numberOfMisMatches > 0)
  {
    MITK_INFO << "Dumping test storage because there were errors:";
    for (auto entry : m_TestNodesByHierarchy)
    {
      const std::string& key = entry.first;
      const mitk::DataNode::Pointer& node = entry.second;
      MITK_INFO << "  Test node '" << node->GetName() << "', hierarchy : " << key;
    }
  }

  return numberOfMisMatches == 0;
}

bool mitk::DataStorageCompare::AreNodesEqual(const mitk::DataNode* reference, const mitk::DataNode* test, double eps, bool verbose)
{
  if (reference == nullptr && test == nullptr)
    return true;

  if (reference == nullptr && test != nullptr)
  {
    if (verbose)
      MITK_WARN << "  Reference node is nullptr, test node is not (type " << test->GetNameOfClass() << ")";
    return false;
  }

  if (reference != nullptr && test == nullptr)
  {
    if (verbose)
      MITK_WARN << "  Test node is nullptr, reference node is not (type " << reference->GetNameOfClass() << ")";
    return false;
  }

  // two real nodes, need to really compare
  return IsDataEqual(reference->GetData(), test->GetData(), eps, verbose);
}

bool mitk::DataStorageCompare::IsDataEqual(const mitk::BaseData* reference, const mitk::BaseData* test, double eps, bool verbose)
{
  // early-out for nullptrs
  if (reference == nullptr && test == nullptr)
    return true;

  if (reference == nullptr && test != nullptr)
  {
    if (verbose)
      MITK_WARN << "  Reference data is nullptr, test data is not (type " << test->GetNameOfClass() << ")";
    return false;
  }

  if (reference != nullptr && test == nullptr)
  {
    if (verbose)
      MITK_WARN << "  Test data is nullptr, reference data is not (type " << reference->GetNameOfClass() << ")";
    return false;
  }

  // two real BaseData objects, need to really compare
  if (reference->GetNameOfClass() != test->GetNameOfClass() )
  {
    if (verbose)
      MITK_WARN << "  Mismatch: Reference data is '" << reference->GetNameOfClass() << "', "
                << "test data is '" << test->GetNameOfClass() << "'";
    return false;
  }
  try
  {
    std::string ldapFilter = std::string("(basedata=") + reference->GetNameOfClass() + "*)";
    std::vector<us::ServiceReference<BaseDataEqual>> comparators = us::GetModuleContext()->GetServiceReferences<BaseDataEqual>(ldapFilter);
    if (comparators.empty())
    {
      // bad, no comparator found, cannot compare
      MITK_ERROR << "Comparison error: no comparator for objects of type '" << reference->GetNameOfClass() << "'";
      return false;
    }
    else if (comparators.size() > 1)
    {
      MITK_WARN << "Comparison warning: multiple comparisons possible for objects of type '" << reference->GetNameOfClass() << "'. Using just one.";
      // bad, multiple comparators, need to add ranking or something
    }

    BaseDataEqual* comparator = us::GetModuleContext()->GetService<BaseDataEqual>( comparators.front() );
    if (!comparator)
    {
      MITK_ERROR << "Service lookup error, cannot get comparator for class " << reference->GetNameOfClass();
    }

    return comparator->AreEqual(reference, test, eps, verbose);
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception during comparison: " << e.what();
    return false;
  }
}

bool mitk::DataStorageCompare::CompareData(double eps, bool verbose)
{
  int numberOfMisMatches = 0;

  for (auto entry : m_RefNodesByHierarchy)
  {
    const std::string& key = entry.first;
    const mitk::DataNode::Pointer& refNode = entry.second;

    unsigned int timesInReference = m_RefNodesByHierarchy.count(key);
    unsigned int timesInTest = m_TestNodesByHierarchy.count(key);

    if (timesInReference == 1 && timesInTest == 1)
    {
      // go on an compare those two
      auto testEntry = m_TestNodesByHierarchy.find(key);
      mitk::DataNode::Pointer testNode = testEntry->second;
      if ( ! AreNodesEqual(refNode, testNode, eps, verbose) )
      {
        ++numberOfMisMatches;
        if ( verbose )
        {
          MITK_WARN << "### Data mismatch problem";
          MITK_WARN << "  Node '" << key << "' did not compare to equal (see warnings above).";
        }
      }
    }
    else
    {
      ++numberOfMisMatches;
      if (verbose)
      {
        MITK_WARN << "### Data mismatch problem";
        MITK_WARN << "  Reference storage has " << timesInReference << " node(s), test storage " << timesInTest;
        MITK_WARN << "  This does not match or we don't know how to figure out comparison partners";
      }
    }
  }

  // TODO factor this out? Should describe data only.
  // can we get that to a higher level and include only
  // problematic data? think about error reporting
  if (verbose && numberOfMisMatches > 0)
  {
    MITK_INFO << "Dumping test storage because there were errors:";
    for (auto entry : m_TestNodesByHierarchy)
    {
      const std::string& key = entry.first;
      const mitk::DataNode::Pointer& node = entry.second;
      MITK_INFO << "  Test node '" << node->GetName() << "', hierarchy : " << key;
    }
  }

  return numberOfMisMatches == 0;
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

