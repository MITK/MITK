/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateData_h
#define mitkNodePredicateData_h

#include <mitkNodePredicateBase.h>

namespace mitk
{
  class BaseData;

  /** \brief Predicate that evaluates if the given DataNode's data object pointer equals a given pointer.
   *
   * NodePredicateData(nullptr) returns true if a DataNode does not have a data object
   * (e.g. GetData() returns nullptr). This could return an unexpected number of nodes
   * (e.g. the root node of the tree).
   *
   * \warning NodePredicateData holds a raw pointer to a BaseData!
   * <tt>NodePredicateData p(mitk::BaseData::New());</tt> will not work because the
   * temporary smart pointer will be destroyed. Intended use is:
   * <tt>NodePredicateData p(myDataObject); result = myDataStorage->GetSubset(p);</tt>
   * Then work with result, do not reuse p later.
   *
   * \ingroup DataStorage
   */
  class MITKCORE_EXPORT NodePredicateData : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateData, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateData, mitk::BaseData *);

    /** \brief Standard Destructor. */
    ~NodePredicateData() override;

    /** \brief Checks if the node's data object pointer equals the stored pointer.
     *
     * \param node The DataNode to check.
     * \return True if node->GetData() equals the stored data object pointer.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    /** \brief Protected constructor, use static instantiation functions instead.
     *
     * \param d Pointer to the data object to compare against.
     */
    NodePredicateData(mitk::BaseData *d);

    mitk::BaseData *m_DataObject;
  };
} // namespace mitk

#endif
