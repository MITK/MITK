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

RdfTriple::RdfTriple(RdfNode x, RdfNode y, RdfNode z)
  : subject(x), predicate(y), object(z)
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
