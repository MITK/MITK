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

namespace mitk {

RdfTriple::RdfTriple()
{
}

RdfTriple::RdfTriple(RdfNode subject, RdfNode predicate, RdfNode object)
  : subject(subject), predicate(predicate), object(object)
{
}

RdfTriple::RdfTriple(RdfNode subject, RdfNode property, std::string value)
  : subject(subject), predicate(property), object(RdfNode(value))
{
}

RdfTriple::~RdfTriple()
{
}

RdfNode RdfTriple::GetSubject() const
{
  return subject;
}

RdfNode RdfTriple::GetPredicate() const
{
  return predicate;
}

RdfNode RdfTriple::GetObject() const
{
  return object;
}

// Define outstream of a Triple
std::ostream & operator<<(std::ostream &out, const RdfTriple &t)
{
  return out << "( " << t.GetSubject() << " " << t.GetPredicate() << " " << t.GetObject() << " )";
}

} // end of namespace mitk
