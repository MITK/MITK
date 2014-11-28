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

#ifndef MITKRDFURI_H
#define MITKRDFURI_H

#include <MitkRDFExports.h>

#include <string>

namespace mitk {

class MitkRDF_EXPORT RdfUri
{
public:
  RdfUri();
  RdfUri(std::string uri);

  virtual ~RdfUri();

  std::string ToString();

  bool operator==(const RdfUri &u) const;
  bool operator!=(const RdfUri &u) const;

private:
  std::string m_Uri;
};

}

#endif // MITKRDFURI_H
