/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateOr_h
#define mitkNodePredicateOr_h

#include <mitkNodePredicateCompositeBase.h>

namespace mitk
{
  /**
   * \brief Composite predicate that forms a logical OR (disjunction) from its child predicates.
   *
   * A node passes the check if at least one child predicate returns true.
   * Evaluation short-circuits on the first true result.
   *
   * \code
   * auto pred = NodePredicateOr::New(
   *   NodePredicateDataType::New("Image"),
   *   NodePredicateDataType::New("Surface"));
   * auto imagesOrSurfaces = dataStorage->GetSubset(pred);
   * \endcode
   *
   * \ingroup DataStorage
   * \sa NodePredicateAnd, NodePredicateNot, NodePredicateCompositeBase
   */
  class MITKCORE_EXPORT NodePredicateOr : public NodePredicateCompositeBase
  {
  public:
    mitkClassMacro(NodePredicateOr, NodePredicateCompositeBase);
    itkFactorylessNewMacro(NodePredicateOr);
    mitkNewMacro2Param(NodePredicateOr, const NodePredicateBase *, const NodePredicateBase *);
    mitkNewMacro3Param(NodePredicateOr, const NodePredicateBase *, const NodePredicateBase *, const NodePredicateBase *);

    /**
     * \brief Destructor.
     */
    ~NodePredicateOr() override;

    /**
     * \brief Check whether the node satisfies any child predicate (logical OR).
     *
     * \param[in] node  The DataNode to evaluate. Must not be nullptr.
     * \return \a true if at least one child predicate returns true for the node.
     * \throw std::invalid_argument if no child predicates have been added or \a node is nullptr.
     */
    bool CheckNode(const DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Constructor
    NodePredicateOr();
    //##Documentation
    //## @brief Convenience constructor that adds p1 and p2 to list of child predicates
    NodePredicateOr(const NodePredicateBase *p1, const NodePredicateBase *p2);
    //##Documentation
    //## @brief Convenience constructor that adds p1, p2 and p3 to list of child predicates
    NodePredicateOr(const NodePredicateBase *p1, const NodePredicateBase *p2, const NodePredicateBase *p3);
  };

} // namespace mitk

#endif
