/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateBase_h
#define mitkNodePredicateBase_h

#include <itkObject.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  class DataNode;
  /**
   * \brief Abstract base class for predicates used to filter DataNode objects in DataStorage queries.
   *
   * Predicates are used with DataStorage::GetSubset() and related methods to select nodes
   * that match specific criteria. By combining predicates with NodePredicateAnd,
   * NodePredicateOr, and NodePredicateNot, complex queries can be formed, such as
   * "give me all nodes that contain a surface object or a binary segmentation and that
   * are tagged as OrganType == 'Liver'".
   *
   * Predicates use the Composite Design Pattern and are derived from itk::Object,
   * thus benefiting from smart pointer memory management.
   *
   * \warning Predicates should only store raw pointers to external objects (not smart pointers),
   *          because predicates are not owners of those objects and should not keep them alive.
   *
   * \ingroup DataStorage
   * \sa NodePredicateAnd, NodePredicateOr, NodePredicateNot, NodePredicateProperty,
   *     NodePredicateDataType, DataStorage::GetSubset
   */
  class MITKCORE_EXPORT NodePredicateBase : public itk::Object
  {
  public:
    mitkClassMacroItkParent(NodePredicateBase, itk::Object);

    /**
     * \brief Virtual destructor.
     */
    ~NodePredicateBase() override;

    /**
     * \brief Evaluate whether the given node satisfies this predicate's condition.
     *
     * Subclasses must implement this method to define their specific evaluation logic.
     *
     * \param[in] node  The DataNode to evaluate. Must not be nullptr.
     * \return \a true if the node satisfies the condition, \a false otherwise.
     */
    virtual bool CheckNode(const mitk::DataNode *node) const = 0;
  };

} // namespace mitk

#endif
