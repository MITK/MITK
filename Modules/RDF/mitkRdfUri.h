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
  /**
  * \ingroup MitkRDFModule
  */
  class MITKRDF_EXPORT RdfUri
  {
  public:

    /**
    * Construct a empty RdfUri.
    */
    RdfUri();

    /**
    * Construct a RdfUri with a value.
    * @param uri A std:string which represents the transfer parameter from a URI.
    */
    explicit RdfUri(std::string uri);

    virtual ~RdfUri();

    /**
    * Returns the string value of an RdfUri.
    * @return Value of RdfUri as std:string.
    */
    std::string ToString() const;

    /**
    * Set the value of an RdfUri.
    * @param uri New value of an RdfUri.
    */
    void SetUri(std::string uri);

    bool operator==(const RdfUri &u) const;
    bool operator!=(const RdfUri &u) const;

  private:
    std::string m_Uri;
  };
}

#endif // MITKRDFURI_H
