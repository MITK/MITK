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

#include "mitkRdfNode.h"

#include <stdio.h>
#include <ostream>
#include <ctype.h>
#include <stdarg.h>

#include <redland.h>

namespace mitk {

  RdfNode::RdfNode()
    : m_Type(NOTHING)
  {
  }

  RdfNode::RdfNode(RdfUri uri)
    : m_Type(URI), m_Value(uri.ToString())
  {
  }

  RdfNode::RdfNode(std::string text)
    : m_Type(LITERAL), m_Value(text)
  {
  }

  RdfNode::RdfNode(std::string text, RdfUri dataType)
    : m_Type(LITERAL), m_Value(text), m_Datatype(dataType)
  {
  }

  RdfNode::~RdfNode()
  {
  }

  void RdfNode::SetType(RdfNode::Type type)
  {
    m_Type = type;
  }

  void RdfNode::SetDatatype(RdfUri dataType)
  {
    m_Datatype = dataType;
  }

  void RdfNode::SetValue(std::string value)
  {
    m_Value = value;
  }

  RdfNode::Type RdfNode::GetType() const
  {
    return m_Type;
  }

  RdfUri RdfNode::GetDatatype() const
  {
    return m_Datatype;
  }

  std::string RdfNode::GetValue() const
  {
    return m_Value;
  }

  bool RdfNode::operator==(const RdfNode &u) const
  {
    if (this->m_Type != u.m_Type) return false;
    if (this->m_Value.compare(u.m_Value) != 0) return false;
    if (this->m_Datatype != u.m_Datatype) return false;
    return true;
  }

  bool RdfNode::operator!=(const RdfNode &u) const
  {
    return !operator==(u);
  }

  // Define outstream of a Node
  std::ostream & operator<<(std::ostream &out, const RdfNode &n)
  {
    switch (n.GetType()) {
    case RdfNode::NOTHING:
      out << "[]";
      break;
    case RdfNode::URI:
      if (n.GetValue() == "") {
        out << "[empty-uri]";
      } else {
        out << "<" << n.GetValue() << ">";
      }
      break;
    case RdfNode::LITERAL:
      out << "\"" << n.GetValue() << "\"";
      if (n.GetDatatype() != RdfUri()) out << "^^" << n.GetDatatype();
      break;
    case RdfNode::BLANK:
      out << "[blank " << n.GetValue() << "]";
      break;
    }
    return out;
  }
}
