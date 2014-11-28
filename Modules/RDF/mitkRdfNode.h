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

#include "MitkRdfUri.h"

#include <stdio.h>
#include <string>

namespace mitk {
  /**
  * \ingroup MitkRDFModule
  */
  class MitkRDF_EXPORT RdfNode
  {
  public:

    enum Type { Nothing, URI, Literal, Blank};

    // Construct a empty Node
    RdfNode();

    RdfNode(RdfUri uri);

    RdfNode(std::string text);

    RdfNode(std::string text, RdfUri dataType);

    virtual ~RdfNode();

    Type type;
    RdfUri datatype;
    std::string value;

    // dummy method for dummy test
    bool dummy();
  };

  MitkRDF_EXPORT std::ostream & operator<<(std::ostream &out, const mitk::RdfNode &n);
}

#endif
