/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateNot_h
#define mitkNodePredicateNot_h

#include <mitkNodePredicateCompositeBase.h>

namespace mitk
{
  /**
   * \brief Composite predicate that negates (logically inverts) its single child predicate.
   *
   * Although derived from NodePredicateCompositeBase, this predicate holds exactly one
   * child predicate at a time. If AddPredicate() is called when a child already exists,
   * the previous child is replaced.
   *
   * \code
   * auto notVisible = NodePredicateNot::New(
   *   NodePredicateProperty::New("visible", BoolProperty::New(true)));
   * auto invisibleNodes = dataStorage->GetSubset(notVisible);
   * \endcode
   *
   * \ingroup DataStorage
   * \sa NodePredicateAnd, NodePredicateOr, NodePredicateCompositeBase
   */
  class MITKCORE_EXPORT NodePredicateNot : public mitk::NodePredicateCompositeBase
  {
  public:
    mitkClassMacro(NodePredicateNot, NodePredicateCompositeBase);
    mitkNewMacro1Param(NodePredicateNot, const mitk::NodePredicateBase *);

    /**
     * \brief Destructor.
     */
    ~NodePredicateNot() override;

    /**
     * \brief Set the child predicate, replacing any existing one.
     *
     * Only one child predicate is allowed for the NOT predicate. If a child
     * already exists, it is removed before the new one is added.
     *
     * \param[in] p  The child predicate to set.
     */
    void AddPredicate(const mitk::NodePredicateBase *p) override;

    /**
     * \brief Check whether the node does NOT satisfy the child predicate condition.
     *
     * \param[in] node  The DataNode to evaluate. Must not be nullptr.
     * \return \a true if the child predicate returns false for the node (i.e., negation).
     * \throw std::invalid_argument if \a node is nullptr.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Constructor
    NodePredicateNot(const mitk::NodePredicateBase *p);
  };

} // namespace mitk

#endif
