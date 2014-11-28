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

class MitkRDF_EXPORT RdfTriple
{
public:
  // Construct a empty Triple
  RdfTriple();

  // Construct a triple with three nodes
  RdfTriple(RdfNode x, RdfNode y, RdfNode z);

  virtual ~RdfTriple();

  RdfNode GetSubject() const;
  RdfNode GetPredicate() const;
  RdfNode GetObject() const;

private:
  RdfNode subject;
  RdfNode predicate;
  RdfNode object;

};

MitkRDF_EXPORT std::ostream & operator<<(std::ostream &out, const RdfTriple &t);

}

#endif // MITKRDFTRIPLE_H
