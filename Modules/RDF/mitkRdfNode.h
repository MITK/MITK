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

#ifndef MITKRDFNODE_H
#define MITKRDFNODE_H

#include <MitkRDFExports.h>

#include "mitkRdfUri.h"

#include <stdio.h>
#include <string>

namespace mitk {
  /**
  * \ingroup MitkRDFModule
  */
  class MITKRDF_EXPORT RdfNode
  {
  public:

    /**
    * Enumeration for node types.
    */
    enum Type { NOTHING, URI, LITERAL, BLANK };

    /**
    * Construct a empty invalid node.
    */
    RdfNode();

    /**
    * Construct a node from type URI which represents a object of the real world.
    * @param uri An RdfUri which represents a URI.
    */
    RdfNode(RdfUri uri);

    /**
    * Construct a node from type LITERAL.
    * @param text A std::string which represents a literal.
    */
    RdfNode(std::string text);

    /**
    * Construct a node from type LITERAL with a specific data type.
    * @param text A std::string which represents a literal.
    * @param dataType An RdfUri which represents a specific data type.
    */
    RdfNode(std::string text, RdfUri dataType);

    virtual ~RdfNode();

    /**
    * Set the type of a Node.
    * @param type An RdfNode::Type which represents a type of a node.
    */
    void SetType(Type type);

    /**
    * Set the data type of a LITERAL Node.
    * @param dataType An RdfUri which represents an URI of a specific data type.
    */
    void SetDatatype(RdfUri dataType);

    /**
    * Set the internal represantation of an URI or a text.
    * @param value A std::string which represents an URI or a text.
    */
    void SetValue(std::string value);

    /**
    * Get the type of a node.
    * @return The type of a node.
    */
    Type GetType() const;

    /**
    * Get the data type of the internal value of a node.
    * @return The data type of the internal value of a node.
    */
    RdfUri GetDatatype() const;

    /**
    * Get the internal value of a node.
    * @return The internal value of a node.
    */
    std::string GetValue() const;

    bool operator==(const RdfNode &u) const;
    bool operator!=(const RdfNode &u) const;

  private:
    Type m_Type;
    std::string m_Value;
    RdfUri m_Datatype;
  };

  MITKRDF_EXPORT std::ostream & operator<<(std::ostream &out, const mitk::RdfNode &n);
}

#endif
