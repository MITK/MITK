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

#ifndef mitkDataStorageCompare_h_included
#define mitkDataStorageCompare_h_included

#include "mitkDataStorage.h"

namespace mitk
{

/**
  Helper class to compare two DataStorages.

  This is purposely _not_ a mitk::Equal() function because
  the main intention is to use this class for I/O testing,
  and because the definition of equal is really not clear
  for two DataStorages.

  In the context of I/O tests, one could want to verify
  (given two storages "reference" and "test")
    * that all nodes from reference are in test (identification of nodes is difficult)
    * all nodes from test are in reference
    * all property lists are identical (types and values)
    * all data is identical (types, values)
    * all mappers are identical (types, not state)
    * all interactors are identical (type)
    * ... ?

  This class offers a number of flags that describe one of
  those aspects. These flags can be combined by the OR
  operator ("|"). The Compare() method will only test those
  aspects.

  Once Compare() has been executed there are a couple of
  methods to query the results or print a small report.

  <b>Finding nodes</b>:
  Identifying nodes to be compares poses a problem. This is
  because we don't have node identifiers but only names.
  Names are not unique and are not even required to exist
  or to non-empty.

  This class solves this identification as follows:
  ...
*/
class DataStorageCompare
{
public:

  typedef enum
  {
    CMP_Nothing        =  0,
    CMP_Hierarchy      =  1,
    CMP_Data           =  2,
    CMP_Properties     =  4,
    CMP_Mappers        =  8,
    CMP_Interactors    = 16,
    CMP_All = 0xfffffff,
  } Tests;

  DataStorageCompare(const DataStorage* reference,
                     const DataStorage* test,
                     Tests flags = CMP_All,
                     double eps = mitk::eps);

  bool CompareVerbose();
  bool Compare(bool verbose = false);

  void Report();

private:

  bool CompareHierarchy(bool verbose);
  bool CompareDataNodes(bool verbose);

  bool AreNodesEqual(const mitk::DataNode* reference, const mitk::DataNode* test, bool verbose = false);
  bool IsDataEqual(const mitk::BaseData* reference, const mitk::BaseData* test, bool verbose = false);
  bool ArePropertyListsEqual(const mitk::DataNode& reference, const mitk::DataNode& test, bool verbose = false);
  bool ArePropertyListsEqual(const mitk::PropertyList& reference, const mitk::PropertyList& test, bool verbose = false);

  double m_Eps;
  Tests m_TestAspects;
  DataStorage::ConstPointer m_ReferenceDS;
  DataStorage::ConstPointer m_TestDS;

  typedef std::multimap<std::string, DataNode::Pointer> HierarchyDescriptorMap;
  HierarchyDescriptorMap m_RefNodesByHierarchy;
  HierarchyDescriptorMap m_TestNodesByHierarchy;

  /// Calculate hierachy descriptors for each node, store thos in the result map.
  void DescribeHierarchyOfNodes(DataStorage::ConstPointer storage, HierarchyDescriptorMap& result);
  std::string GenerateNodeDescriptor(mitk::DataNode::Pointer node);
  std::string GenerateHierarchyDescriptor(DataNode::Pointer node, DataStorage::ConstPointer storage);

  bool m_HierarchyPassed;
  bool m_DataPassed;
  bool m_PropertiesPassed;
  bool m_MappersPassed;
  bool m_InteractorsPassed;
  int m_AspectsFailed;

}; // class

inline DataStorageCompare::Tests operator|(DataStorageCompare::Tests a, DataStorageCompare::Tests b)
{
  return static_cast<DataStorageCompare::Tests>(static_cast<int>(a) | static_cast<int>(b));
}


} // namespace

#endif
