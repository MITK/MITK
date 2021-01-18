/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataStorageCompare.h"
#include "mitkBaseDataCompare.h"

#include "mitkBaseRenderer.h"
#include "mitkMapper.h"

#include "usGetModuleContext.h"
#include "usLDAPFilter.h"
#include "usModuleContext.h"

mitk::DataStorageCompare::DataStorageCompare(const mitk::DataStorage *reference,
                                             const mitk::DataStorage *test,
                                             Tests flags,
                                             double eps)
  : m_Eps(eps),
    m_TestAspects(flags),
    m_ReferenceDS(reference),
    m_TestDS(test),
    m_HierarchyPassed(true),
    m_DataPassed(true),
    m_PropertiesPassed(true),
    m_MappersPassed(true),
    m_InteractorsPassed(true),
    m_AspectsFailed(0)
{
  BaseDataCompare::RegisterCoreEquals();
}

bool mitk::DataStorageCompare::CompareVerbose()
{
  return Compare(true);
}

bool mitk::DataStorageCompare::Compare(bool verbose)
{
  DescribeHierarchyOfNodes(m_ReferenceDS, m_RefNodesByHierarchy);
  DescribeHierarchyOfNodes(m_TestDS, m_TestNodesByHierarchy);

  m_HierarchyPassed = true;
  m_DataPassed = true;
  m_PropertiesPassed = true;
  m_MappersPassed = true;
  m_InteractorsPassed = true;
  m_AspectsFailed = 0;

  if (m_TestAspects & CMP_Hierarchy)
    m_HierarchyPassed = CompareHierarchy(verbose);

  if (m_TestAspects != CMP_Nothing)
    CompareDataNodes(verbose);

  if ((m_TestAspects & CMP_Data) && !m_DataPassed)
    ++m_AspectsFailed;
  if ((m_TestAspects & CMP_Properties) && !m_PropertiesPassed)
    ++m_AspectsFailed;
  if ((m_TestAspects & CMP_Mappers) && !m_MappersPassed)
    ++m_AspectsFailed;
  if ((m_TestAspects & CMP_Interactors) && !m_InteractorsPassed)
    ++m_AspectsFailed;

  if (verbose)
    Report();

  return m_AspectsFailed == 0;
}

void mitk::DataStorageCompare::Report()
{
  MITK_INFO << "Comparison results:";
  MITK_INFO << "  Hierarchy comparison:   "
            << (m_TestAspects & CMP_Hierarchy ? (m_HierarchyPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Data comparison:        "
            << (m_TestAspects & CMP_Data ? (m_DataPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Properties comparison:  "
            << (m_TestAspects & CMP_Properties ? (m_PropertiesPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Mappers comparison:     "
            << (m_TestAspects & CMP_Mappers ? (m_MappersPassed ? "pass" : "fail") : "skipped");
  MITK_INFO << "  Interactors comparison: "
            << (m_TestAspects & CMP_Interactors ? (m_InteractorsPassed ? "pass" : "fail") : "skipped");

  if (m_AspectsFailed == 0)
    MITK_INFO << "  Summary:                 ALL PASSED";
  else
    MITK_INFO << "  Summary:                " << m_AspectsFailed << " failures";
}

void mitk::DataStorageCompare::DescribeHierarchyOfNodes(DataStorage::ConstPointer storage,
                                                        HierarchyDescriptorMap &result)
{
  result.clear();
  if (storage.IsNull())
    return;

  mitk::DataStorage::SetOfObjects::ConstPointer allNodes = storage->GetAll();
  for (auto node : *allNodes)
  {
    std::string descriptor = GenerateHierarchyDescriptor(node, storage);
    result.insert(std::make_pair(descriptor, node));
  }
}

std::string mitk::DataStorageCompare::GenerateNodeDescriptor(mitk::DataNode::Pointer node)
{
  if (node.IsNull())
    return "nullptr";

  std::string thisDataDescriptor = "nullptr";
  mitk::BaseData *data = node->GetData();
  if (data != nullptr)
    thisDataDescriptor = data->GetNameOfClass();

  std::string thisNodeName = node->GetName();

  std::string thisNodesDescriptor = std::string("_") + thisDataDescriptor + "_(" + thisNodeName + ")";
  return thisNodesDescriptor;
}

std::string mitk::DataStorageCompare::GenerateHierarchyDescriptor(mitk::DataNode::Pointer node,
                                                                  mitk::DataStorage::ConstPointer storage)
{
  std::string thisNodesDescriptor = GenerateNodeDescriptor(node);
  mitk::DataStorage::SetOfObjects::ConstPointer parents =
    storage->GetSources(node, nullptr, true); // direct sources without filter

  // construct descriptors for parents
  std::vector<std::string> parentDescriptors;

  for (const auto &parent : *parents)
    parentDescriptors.push_back(GenerateHierarchyDescriptor(parent, storage));

  // sort descriptors (we don't want to rely on potentially random order of parents)
  std::sort(parentDescriptors.begin(), parentDescriptors.end());

  // construct a string from all sorted parent descriptors
  if (!parentDescriptors.empty())
  {
    thisNodesDescriptor += " <(";
    for (const auto &descriptor : parentDescriptors)
    {
      if (descriptor != parentDescriptors.front()) // join by '+'
      {
        thisNodesDescriptor += " + ";
      }
      thisNodesDescriptor += descriptor;
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
  for (const auto &entry : m_RefNodesByHierarchy)
  {
    const std::string &key = entry.first;
    const mitk::DataNode::Pointer &node = entry.second;

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
  for (const auto &entry : m_TestNodesByHierarchy)
  {
    const std::string &key = entry.first;
    const mitk::DataNode::Pointer &node = entry.second;

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
        MITK_WARN << "  Test storage has more nodes (" << timesInReference << ") than reference storage ("
                  << timesInTest << ")";
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
    MITK_WARN << "Dumping test storage because there were errors:";
    for (const auto &entry : m_TestNodesByHierarchy)
    {
      const std::string &key = entry.first;
      const mitk::DataNode::Pointer &node = entry.second;
      MITK_WARN << "  Test node '" << node->GetName() << "', hierarchy : " << key;
    }
  }

  return numberOfMisMatches == 0;
}

bool mitk::DataStorageCompare::AreNodesEqual(const mitk::DataNode *reference, const mitk::DataNode *test, bool verbose)
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

  if (m_TestAspects & CMP_Data)
    m_DataPassed &= IsDataEqual(reference->GetData(), test->GetData(), verbose);

  if (m_TestAspects & CMP_Properties)
    m_PropertiesPassed &= ArePropertyListsEqual(*reference, *test, verbose);

  if (m_TestAspects & CMP_Mappers)
    m_MappersPassed &= AreMappersEqual(*reference, *test, verbose);

  // .. add interactors/mappers

  // two real nodes, need to really compare
  return m_AspectsFailed == 0;
}

bool mitk::DataStorageCompare::IsDataEqual(const mitk::BaseData *reference, const mitk::BaseData *test, bool verbose)
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
  if (reference->GetNameOfClass() != test->GetNameOfClass())
  {
    if (verbose)
      MITK_WARN << "  Mismatch: Reference data is '" << reference->GetNameOfClass() << "', "
                << "test data is '" << test->GetNameOfClass() << "'";
    return false;
  }
  try
  {
    std::string ldapFilter = std::string("(basedata=") + reference->GetNameOfClass() + "*)";
    std::vector<us::ServiceReference<BaseDataCompare>> comparators =
      us::GetModuleContext()->GetServiceReferences<BaseDataCompare>(ldapFilter);
    if (comparators.empty())
    {
      // bad, no comparator found, cannot compare
      MITK_ERROR << "Comparison error: no comparator for objects of type '" << reference->GetNameOfClass() << "'";
      return false;
    }
    else if (comparators.size() > 1)
    {
      MITK_WARN << "Comparison warning: multiple comparisons possible for objects of type '"
                << reference->GetNameOfClass() << "'. Using just one.";
      // bad, multiple comparators, need to add ranking or something
    }

    auto *comparator = us::GetModuleContext()->GetService<BaseDataCompare>(comparators.front());
    if (!comparator)
    {
      MITK_ERROR << "Service lookup error, cannot get comparator for class " << reference->GetNameOfClass();
    }

    return comparator->AreEqual(reference, test, m_Eps, verbose);
  }
  catch (std::exception &e)
  {
    MITK_ERROR << "Exception during comparison: " << e.what();
    return false;
  }
}

bool mitk::DataStorageCompare::ArePropertyListsEqual(const mitk::DataNode &reference,
                                                     const mitk::DataNode &test,
                                                     bool verbose)
{
  DataNode::PropertyListKeyNames refListNames = reference.GetPropertyListNames();
  DataNode::PropertyListKeyNames testListNames = test.GetPropertyListNames();
  // add the empty names to treat all lists equally
  refListNames.push_back("");
  testListNames.push_back("");

  // verify that list names are identical
  bool error = false;
  if (refListNames.size() != testListNames.size())
  {
    for (const auto &name : refListNames)
      if (std::find(testListNames.begin(), testListNames.end(), name) == testListNames.end())
      {
        MITK_WARN << "Propertylist '" << name << "' from reference node (" << reference.GetName()
                  << ") not found in test node.";
        error = true;
      }

    for (const auto &name : testListNames)
      if (std::find(refListNames.begin(), refListNames.end(), name) == refListNames.end())
      {
        MITK_WARN << "Propertylist '" << name << "' did not exist in reference node (" << reference.GetName()
                  << "), but is present in test node.";
        error = true;
      }

    if (error)
      return false;
  }

  // compare each list
  for (const auto &name : refListNames)
  {
    if (!ArePropertyListsEqual(*(reference.GetPropertyList(name)), *(test.GetPropertyList(name)), verbose))
    {
      MITK_WARN << "Property mismatch while comparing propertylist '" << name << "'. See messages above.";
      error = true;
    }
  }

  return !error;
}

bool mitk::DataStorageCompare::ArePropertyListsEqual(const mitk::PropertyList &reference,
                                                     const mitk::PropertyList &test,
                                                     bool verbose)
{
  const mitk::PropertyList::PropertyMap *refMap = reference.GetMap();

  bool error = false;

  for (const auto &refEntry : *refMap)
  {
    std::string propertyKey = refEntry.first;
    BaseProperty::Pointer refProperty = refEntry.second;
    BaseProperty::Pointer testProperty = test.GetProperty(propertyKey);

    if (testProperty.IsNull())
    {
      if (verbose)
        MITK_WARN << "Property '" << propertyKey << "' not found in test, only in reference.";
      error = true;
    }
    else
    {
      if (!(*refProperty == *testProperty))
      {
        if (verbose)
        {
          MITK_WARN << "Property '" << propertyKey << "' does not match original.";
          MITK_WARN << "Reference was: " << refProperty->GetValueAsString();
          MITK_WARN << "Test was:" << testProperty->GetValueAsString();
        }
        error = true;
      }
    }
  }

  return !error;
}

bool mitk::DataStorageCompare::AreMappersEqual(const mitk::DataNode &reference,
                                               const mitk::DataNode &test,
                                               bool verbose)
{
  bool error = false;

  mitk::Mapper *refMapper2D = reference.GetMapper(mitk::BaseRenderer::Standard2D);
  mitk::Mapper *testMapper2D = test.GetMapper(mitk::BaseRenderer::Standard2D);

  if (refMapper2D == nullptr && testMapper2D == nullptr)
  {
    ; // ok
  }
  else if (refMapper2D != nullptr && testMapper2D == nullptr)
  {
    if (verbose)
    {
      MITK_WARN << "Mapper for 2D was '" << refMapper2D->GetNameOfClass() << "' in reference, is 'nullptr"
                << "' in test (DataNode '" << reference.GetName() << "')";
    }
    error = true;
  }
  else if (refMapper2D == nullptr && testMapper2D != nullptr)
  {
    if (verbose)
    {
      MITK_WARN << "Mapper for 2D was 'nullptr"
                << "' in reference, is '" << testMapper2D->GetNameOfClass() << "' in test (DataNode '"
                << reference.GetName() << "')";
    }
    error = true;
  } // else both are valid pointers, we just compare the type
  else if (refMapper2D->GetNameOfClass() != testMapper2D->GetNameOfClass())
  {
    if (verbose)
    {
      MITK_WARN << "Mapper for 2D was '" << refMapper2D->GetNameOfClass() << "' in reference, is '"
                << testMapper2D->GetNameOfClass() << "' in test (DataNode '" << reference.GetName() << "')";
    }
    error = true;
  }

  mitk::Mapper *refMapper3D = reference.GetMapper(mitk::BaseRenderer::Standard3D);
  mitk::Mapper *testMapper3D = test.GetMapper(mitk::BaseRenderer::Standard3D);

  if (refMapper3D == nullptr && testMapper3D == nullptr)
  {
    ; // ok
  }
  else if (refMapper3D != nullptr && testMapper3D == nullptr)
  {
    if (verbose)
    {
      MITK_WARN << "Mapper for 3D was '" << refMapper3D->GetNameOfClass() << "' in reference, is 'nullptr"
                << "' in test (DataNode '" << reference.GetName() << "')";
    }
    error = true;
  }
  else if (refMapper3D == nullptr && testMapper3D != nullptr)
  {
    if (verbose)
    {
      MITK_WARN << "Mapper for 3D was 'nullptr"
                << "' in reference, is '" << testMapper3D->GetNameOfClass() << "' in test (DataNode '"
                << reference.GetName() << "')";
    }
    error = true;
  } // else both are valid pointers, we just compare the type
  else if (refMapper3D->GetNameOfClass() != testMapper3D->GetNameOfClass())
  {
    if (verbose)
    {
      MITK_WARN << "Mapper for 3D was '" << refMapper3D->GetNameOfClass() << "' in reference, is '"
                << testMapper3D->GetNameOfClass() << "' in test (DataNode '" << reference.GetName() << "')";
    }
    error = true;
  }

  return !error;
}

bool mitk::DataStorageCompare::CompareDataNodes(bool verbose)
{
  int numberOfMisMatches = 0;

  for (const auto &entry : m_RefNodesByHierarchy)
  {
    const std::string &key = entry.first;
    const mitk::DataNode::Pointer &refNode = entry.second;

    unsigned int timesInReference = m_RefNodesByHierarchy.count(key);
    unsigned int timesInTest = m_TestNodesByHierarchy.count(key);

    if (timesInReference == 1 && timesInTest == 1)
    {
      // go on an compare those two
      auto testEntry = m_TestNodesByHierarchy.find(key);
      mitk::DataNode::Pointer testNode = testEntry->second;
      if (!AreNodesEqual(refNode, testNode, verbose))
      {
        ++numberOfMisMatches;
        if (verbose)
        {
          MITK_WARN << "### DataNode mismatch problem";
          MITK_WARN << "  Node '" << key << "' did not compare to equal (see warnings above).";
        }
      }
    }
    else
    {
      ++numberOfMisMatches;
      if (verbose)
      {
        MITK_WARN << "### DataNode mismatch problem";
        MITK_WARN << "  Reference storage has " << timesInReference << " node(s), test storage " << timesInTest;
        MITK_WARN << "  This does not match or we don't know how to figure out comparison partners";
      }
    }
  }

  return numberOfMisMatches == 0;
}
