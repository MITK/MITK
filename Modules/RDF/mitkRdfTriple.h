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

#ifndef MITKRDFTRIPLE_H
#define MITKRDFTRIPLE_H

#include <MitkRDFExports.h>

#include "mitkRdfNode.h"

namespace mitk {
  /**
  * \ingroup MitkRDFModule
  */
  class MITKRDF_EXPORT RdfTriple
  {
  public:

    /**
    * Construct a empty invalid triple.
    */
    RdfTriple();

    /**
    * Construct a normal triple with two nodes and an object property node between.
    * @param subject A node.
    * @param predicate A node which represents an object property.
    * @param object A node.
    */
    RdfTriple(RdfNode subject, RdfNode predicate, RdfNode object);

    /**
    * Construct a normal triple with a node, a data property and a value.
    * @param subject A node.
    * @param predicate A node which represents a data property.
    * @param value A text value.
    */
    RdfTriple(RdfNode subject, RdfNode property, std::string value);

    virtual ~RdfTriple();

    /**
    * Set the subject of a triple.
    * @param subject A node from type URI or BLANK.
    */
    void SetTripleSubject(RdfNode subject);

    /**
    * Set the predicate of a triple.
    * @param predicate A node from type URI which can represent an object property or a data property.
    */
    void SetTriplePredicate(RdfNode predicate);

    /**
    * Set the object of a triple with an object property.
    * @param object A node from type URI, BLANK or LITERAL.
    */
    void SetTripleObject(RdfNode object);

    /**
    * Set the object of a triple with a data property as text.
    * @param text A literal value..
    */
    void SetTripleObject(std::string text);

    /**
    * Get the subject of a triple.
    * @return The subject of a triple.
    */
    RdfNode GetTripleSubject() const;

    /**
    * Get the predicate of a triple.
    * @return The predicate of a triple.
    */
    RdfNode GetTriplePredicate() const;

    /**
    * Get the object of a triple.
    * @return The object of a triple.
    */
    RdfNode GetTripleObject() const;

    bool operator==(const RdfTriple &u) const;
    bool operator!=(const RdfTriple &u) const;

  private:
    RdfNode m_Subject;
    RdfNode m_Predicate;
    RdfNode m_Object;
  };

  MITKRDF_EXPORT std::ostream & operator<<(std::ostream &out, const RdfTriple &t);
}

#endif // MITKRDFTRIPLE_H
