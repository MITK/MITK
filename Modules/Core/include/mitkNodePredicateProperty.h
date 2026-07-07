/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateProperty_h
#define mitkNodePredicateProperty_h

#include <mitkBaseProperty.h>
#include <mitkBaseRenderer.h>
#include <mitkNodePredicateBase.h>

namespace mitk
{
  /**
   * \brief Predicate that evaluates whether a DataNode has a specific property.
   *
   * If only a property name is specified (second constructor parameter is nullptr),
   * the predicate checks for the existence of any property with that name.
   * If a property value is also provided, the predicate checks that the node's
   * property matches the given value (using operator==).
   *
   * If a renderer is specified, the renderer-specific property is checked.
   * Otherwise, the renderer-independent property is checked.
   *
   * \ingroup DataStorage
   * \sa NodePredicateDataProperty, NodePredicateBase, DataNode::GetProperty
   */
  class MITKCORE_EXPORT NodePredicateProperty : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateProperty, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateProperty, const char *);
    mitkNewMacro2Param(NodePredicateProperty, const char *, mitk::BaseProperty *);
    mitkNewMacro3Param(NodePredicateProperty, const char *, mitk::BaseProperty *, const mitk::BaseRenderer *);

    /**
     * \brief Destructor.
     */
    ~NodePredicateProperty() override;

    /**
     * \brief Check whether the node has the expected property.
     *
     * If m_ValidProperty is nullptr, only the existence of a property with the
     * configured name is checked. Otherwise, both name and value equality are verified.
     *
     * \param[in] node  The DataNode to evaluate. Must not be nullptr.
     * \return \a true if the property exists (and matches the expected value, if set).
     * \throw std::invalid_argument if \a node is nullptr or the property name is empty.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    //##Documentation
    //## @brief Constructor to check for a named property
    NodePredicateProperty(const char *propertyName,
                          mitk::BaseProperty *p = nullptr,
                          const mitk::BaseRenderer *renderer = nullptr);

    mitk::BaseProperty::Pointer m_ValidProperty;
    // mitk::BaseProperty* m_ValidProperty;
    std::string m_ValidPropertyName;
    const mitk::BaseRenderer *m_Renderer;
  };

} // namespace mitk

#endif
