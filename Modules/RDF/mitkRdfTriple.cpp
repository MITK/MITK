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

#include "mitkRdfTriple.h"

#include <ostream>

namespace mitk {
  RdfTriple::RdfTriple()
  {
  }

  RdfTriple::RdfTriple(RdfNode subject, RdfNode predicate, RdfNode object)
    : m_Subject(subject), m_Predicate(predicate), m_Object(object)
  {
  }

  RdfTriple::RdfTriple(RdfNode subject, RdfNode property, std::string value)
    : m_Subject(subject), m_Predicate(property), m_Object(RdfNode(value))
  {
  }

  RdfTriple::~RdfTriple()
  {
  }

  void RdfTriple::SetTripleSubject(RdfNode subject)
  {
    m_Subject = subject;
  }

  void RdfTriple::SetTriplePredicate(RdfNode predicate)
  {
    m_Predicate = predicate;
  }

  void RdfTriple::SetTripleObject(RdfNode object)
  {
    m_Object = object;
  }

  void RdfTriple::SetTripleObject(std::string text)
  {
    mitk::RdfNode object = mitk::RdfNode(text);
    m_Object = object;
  }

  RdfNode RdfTriple::GetTripleSubject() const
  {
    return m_Subject;
  }

  RdfNode RdfTriple::GetTriplePredicate() const
  {
    return m_Predicate;
  }

  RdfNode RdfTriple::GetTripleObject() const
  {
    return m_Object;
  }

  bool RdfTriple::operator==(const RdfTriple &u) const
  {
    if (this->m_Subject != u.m_Subject) return false;
    if (this->m_Predicate != m_Predicate) return false;
    if (this->m_Object != u.m_Object) return false;
    return true;
  }

  bool RdfTriple::operator!=(const RdfTriple &u) const
  {
    return !operator==(u);
  }

  // Define outstream of a Triple
  std::ostream & operator<<(std::ostream &out, const RdfTriple &t)
  {
    return out << "( " << t.GetTripleSubject() << " " << t.GetTriplePredicate() << " " << t.GetTripleObject() << " )";
  }
} // end of namespace mitk
