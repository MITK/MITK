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

#ifndef MITKNODEPREDICATEPROPERTY_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEPROPERTY_H_HEADER_INCLUDED_

#include "mitkBaseProperty.h"
#include "mitkBaseRenderer.h"
#include "mitkNodePredicateBase.h"

namespace mitk
{
  //##Documentation
  //## @brief Predicate that evaluates if the given DataNode has a specific property.
  //## If the second parameter is nullptr, it will only be checked whether there is a property with the specified name.
  //## If a renderer is specified in the third parameter the renderer-specific property will be checked. If this
  //## parameter is nullptr or not specified, then the non-renderer-specific property will be checked.
  //##
  //##
  //##
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateProperty : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateProperty, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateProperty, const char *);
    mitkNewMacro2Param(NodePredicateProperty, const char *, mitk::BaseProperty *);
    mitkNewMacro3Param(NodePredicateProperty, const char *, mitk::BaseProperty *, const mitk::BaseRenderer *);

    //##Documentation
    //## @brief Standard Destructor
    ~NodePredicateProperty() override;

    //##Documentation
    //## @brief Checks, if the nodes contains a property that is equal to m_ValidProperty
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

#endif /* MITKNODEPREDICATEPROPERTY_H_HEADER_INCLUDED_ */
