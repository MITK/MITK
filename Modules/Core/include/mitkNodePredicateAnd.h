/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateAnd_h
#define mitkNodePredicateAnd_h

#include <mitkNodePredicateCompositeBase.h>

namespace mitk
{
  /**
   * \brief Composite predicate that forms a logical AND (conjunction) from its child predicates.
   *
   * A node passes the check if and only if all child predicates return true.
   * Child predicates are evaluated in order; evaluation short-circuits on the first false result.
   *
   * \code
   * auto pred = NodePredicateAnd::New(
   *   NodePredicateDataType::New("Image"),
   *   NodePredicateProperty::New("visible", BoolProperty::New(true)));
   * auto visibleImages = dataStorage->GetSubset(pred);
   * \endcode
   *
   * \ingroup DataStorage
   * \sa NodePredicateOr, NodePredicateNot, NodePredicateCompositeBase
   */
  class MITKCORE_EXPORT NodePredicateAnd : public NodePredicateCompositeBase
  {
  public:
    mitkClassMacro(NodePredicateAnd, NodePredicateCompositeBase);

    itkFactorylessNewMacro(NodePredicateAnd);
    mitkNewMacro2Param(NodePredicateAnd, const NodePredicateBase *, const NodePredicateBase *);
    mitkNewMacro3Param(NodePredicateAnd,
                       const NodePredicateBase *,
                       const NodePredicateBase *,
                       const NodePredicateBase *);

    /**
     * \brief Destructor.
     */
    ~NodePredicateAnd() override;

    /**
     * \brief Check whether the node satisfies all child predicates (logical AND).
     *
     * \param[in] node  The DataNode to evaluate. Must not be nullptr.
     * \return \a true if all child predicates return true for the node.
     * \throw std::invalid_argument if no child predicates have been added or \a node is nullptr.
     */
    bool CheckNode(const DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Protected constructor, use static instantiation functions instead
    NodePredicateAnd();

    //##Documentation
    //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
    //## Protected constructor, use static instantiation functions instead
    NodePredicateAnd(const NodePredicateBase *p1, const NodePredicateBase *p2);

    //##Documentation
    //## @brief Convenience constructor that adds p1, p2 and p3 to list of child predicates
    //## Protected constructor, use static instantiation functions instead
    NodePredicateAnd(const NodePredicateBase *p1, const NodePredicateBase *p2, const NodePredicateBase *p3);
  };

} // namespace mitk

#endif
