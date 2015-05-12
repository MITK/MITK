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

#include "mitkRdfUri.h"

namespace mitk {
  RdfUri::RdfUri()
  {
  }

  RdfUri::RdfUri(std::string uri)
    : m_Uri(uri)
  {
  }

  RdfUri::~RdfUri()
  {
  }

  std::string RdfUri::ToString() const
  {
    return m_Uri;
  }

  void RdfUri::SetUri(std::string uri)
  {
    m_Uri = uri;
  }

  bool RdfUri::operator==(const RdfUri &u) const
  {
    if (this->m_Uri.compare(u.m_Uri) != 0) return false;
    return true;
  }

  bool RdfUri::operator!=(const RdfUri &u) const
  {
    return !operator==(u);
  }
} // end of namespace mitk
