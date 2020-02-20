/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    or to be non-empty.

    This class does not solve this problem completely. It
    identifies nodes by constructing an "identifier" that
    includes the node name and the BaseData class name of
    the node itself and all its direct and indirect parents.

    This leaves certain cases where two nodes in the hierarchy
    would get the same identifier. For the use in test cases
    (\sa SceneIOTestScenarioProvider) this is sufficient, however,
    since we can make sure that each node gets an individual name.
  */
  class DataStorageCompare
  {
  public:
    /**
     * \brief Flag describing the aspects of comparing two DataStorages.
     *
     * Flag values can be combined by the "|" operator (bitwise or).
     */
    typedef enum {
      CMP_Nothing = 0,      ///< No tests
      CMP_Hierarchy = 1,    ///< Compare hierarchy
      CMP_Data = 2,         ///< Compare BaseData
      CMP_Properties = 4,   ///< Compare PropertyLists
      CMP_Mappers = 8,      ///< Compare Mapper types
      CMP_Interactors = 16, ///< Compare interactors
      CMP_All = 0xfffffff,  ///< Compare all known aspects
    } Tests;

    /**
     * \brief Constructor taking reference and test DataStorage.
     *
     * \param reference Reference DataStorage object.
     * \param test Test DataStorage object.
     * \param flags Flags describing the desired test aspects (\sa Tests).
     * \param eps precision for floating point comparisons.
     */
    DataStorageCompare(const DataStorage *reference,
                       const DataStorage *test,
                       Tests flags = CMP_All,
                       double eps = mitk::eps);

    /**
     * \brief Shorthand for Compare(true).
     */
    bool CompareVerbose();

    /**
     * \brief Execute the comparison.
     *
     * \param verbose if true, the comparison will output messages for all differences found and Report() will be called
     * at the end.
     * \return true if reference and test are judged equal regarding all flags provided in constructor.
     */
    bool Compare(bool verbose = false);

    /**
     * \brief Prints a small summary of what tests have been executed and which ones failed or passed.
     *
     * Called automatically by Compare() when that method is called with the verbose flag.
     */
    void Report();

  private:
    /**
     * \brief Compares that the structure of nodes is identical.
     *
     * See class comment on identifying nodes for details.
     *
     * Tests both directions: test nodes included in reference
     * storage and reference nodes included in test storage.
     */
    bool CompareHierarchy(bool verbose);

    /**
     * \brief Compares pairs of DataNodes to each other.
     *
     * See class comment on identifying nodes for details.
     *
     * Identifies nodes pairs and calls AreNodesEqual() for
     * pairs that are clearly identified. Warnings are issues
     * for unclear cases.
     */
    bool CompareDataNodes(bool verbose);

    /**
     * \brief Called for each pair of nodes, tests all aspects asked for in constructor.
     *
     * Receives a reference and a test DataNode and executes all comparisons
     * that where required during construction of this DataStorageCompare
     * via the Tests flags.
     *
     * Calls to specific methods for each type of comparison.
     *
     * \sa IsDataEqual()
     * \sa ArePropertyListsEqual()
     * \sa AreMappersEqual()
     */
    bool AreNodesEqual(const mitk::DataNode *reference, const mitk::DataNode *test, bool verbose = false);

    /**
     * \brief Compares two BaseData instances.
     *
     * Makes use of the BaseDataEqual services to compare objects.
     */
    bool IsDataEqual(const mitk::BaseData *reference, const mitk::BaseData *test, bool verbose = false);

    /**
     * \brief Compares all property lists of given nodes.
     *
     * Tests presence of all render specific and default lists,
     * then compares them via the other method named ArePropertyListsEqual().
     */
    bool ArePropertyListsEqual(const mitk::DataNode &reference, const mitk::DataNode &test, bool verbose = false);

    /**
     * \brief Compares the mapper types(!) of given nodes.
     *
     */
    bool AreMappersEqual(const mitk::DataNode &reference, const mitk::DataNode &test, bool verbose);

    /**
     * \brief Compares two instances of PropertyList.
     *
     * Tests if all properties are found in both lists.
     * Tests equalness of properties via BaseProperty::operator==().
     */
    bool ArePropertyListsEqual(const mitk::PropertyList &reference,
                               const mitk::PropertyList &test,
                               bool verbose = false);

    /// Precision/Epsilon used for certain floating point comparisons.
    double m_Eps;

    /// Flags describing what to compare.
    Tests m_TestAspects;

    /// Reference data storage.
    DataStorage::ConstPointer m_ReferenceDS;

    /// Test data storage.
    DataStorage::ConstPointer m_TestDS;

    /**
     * \brief Structure associating "hierachy descriptors" to DataNodes.
     *
     * The string keys are "hierarchy descriptors" which are
     * built by GenerateHierarchyDescriptor() for individual nodes.
     */
    typedef std::multimap<std::string, DataNode::Pointer> HierarchyDescriptorMap;

    /// Structure of the reference storage, filled by Compare().
    HierarchyDescriptorMap m_RefNodesByHierarchy;

    /// Structure of the test storage, filled by Compare().
    HierarchyDescriptorMap m_TestNodesByHierarchy;

    /**
     * \brief Calculate hierachy descriptors for each node, store them in the result map.
     *
     */
    void DescribeHierarchyOfNodes(DataStorage::ConstPointer storage, HierarchyDescriptorMap &result);

    /**
     * \brief Generate descriptor for one single node.
     *
     */
    std::string GenerateNodeDescriptor(mitk::DataNode::Pointer node);

    /**
     * \brief Generate descriptor for the complete hierarchy of a node.
     *
     * \param node DataNode to describe.
     * \param storage DataStorage to find out about the node's parents.
     */
    std::string GenerateHierarchyDescriptor(DataNode::Pointer node, DataStorage::ConstPointer storage);

    bool m_HierarchyPassed;
    bool m_DataPassed;
    bool m_PropertiesPassed;
    bool m_MappersPassed;
    bool m_InteractorsPassed;
    int m_AspectsFailed;

  }; // class

  /// Needed to make the flag DataStorageCompare::Tests usable.
  inline DataStorageCompare::Tests operator|(DataStorageCompare::Tests a, DataStorageCompare::Tests b)
  {
    return static_cast<DataStorageCompare::Tests>(static_cast<int>(a) | static_cast<int>(b));
  }

} // namespace

#endif
