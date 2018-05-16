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

#include <algorithm>
#include <utility>

#include <mitkExceptionMacro.h>
#include <mitkPropertyKeyPath.h>

#include <regex>

namespace mitk
{
  PropertyKeyPath::NodeInfo::NodeInfo() : type(NodeType::Invalid), selection(0){};

  PropertyKeyPath::NodeInfo::NodeInfo(const ElementNameType &name, NodeType type, ItemSelectionIndex index)
    : type(type), name(name), selection(index){};

  bool PropertyKeyPath::NodeInfo::operator==(const NodeInfo &right) const
  {
    if (this->name != right.name)
      return false;
    if (this->type != right.type)
      return false;
    if (this->selection != right.selection)
      return false;

    return true;
  };

  bool PropertyKeyPath::NodeInfo::Matches(const NodeInfo &right) const
  {
    if (type == NodeType::Invalid || right.type == NodeType::Invalid)
    {
      return false;
    }
    else if (type == NodeType::AnyElement || right.type == NodeType::AnyElement)
    {
      return true;
    }
    else if (name == right.name)
    {
      if (type == NodeType::Element && right.type == NodeType::Element)
      {
        return true;
      }
      else if (selection == right.selection || type == NodeType::AnySelection || right.type == NodeType::AnySelection)
      {
        return true;
      }
    }
    return false;
  };

  bool PropertyKeyPath::IsEmpty() const { return m_NodeInfos.empty(); };

  bool PropertyKeyPath::IsExplicit() const
  {
    for (const auto &pos : m_NodeInfos)
    {
      if ((pos.type == NodeInfo::NodeType::AnySelection) || (pos.type == NodeInfo::NodeType::AnyElement))
      {
        return false;
      }
    }

    return true;
  };

  bool PropertyKeyPath::HasItemSelectionWildcardsOnly() const
  {
    bool result = false;
    for (const auto &pos : m_NodeInfos)
    {
      if (pos.type == NodeInfo::NodeType::AnyElement)
      {
        return false;
      }
      result = result || pos.type == NodeInfo::NodeType::AnySelection;
    }

    return result;
  };

  PropertyKeyPath::PathIndexType PropertyKeyPath::GetSize() const { return m_NodeInfos.size(); }

  PropertyKeyPath::PathIndexType PropertyKeyPath::AddNode(const NodeInfo &newNode)
  {
    m_NodeInfos.push_back(newNode);
    return m_NodeInfos.size() - 1;
  };

  const PropertyKeyPath::NodeInfo &PropertyKeyPath::GetNode(const PathIndexType &index) const
  {
    if (index >= GetSize())
    {
      mitkThrowException(InvalidPathNodeException)
        << "Error. Cannot return info of path node. Node index is out of bounds. Index: " << index
        << "; Path: " << PropertyKeyPathToPropertyName(*this);
    }

    return m_NodeInfos[index];
  };

  PropertyKeyPath::NodeInfo &PropertyKeyPath::GetNode(const PathIndexType &index)
  {
    if (index >= this->GetSize())
    {
      mitkThrowException(InvalidPathNodeException)
        << "Error. Cannot return info of path node. Node index is out of bounds. Index: " << index
        << "; Path: " << PropertyKeyPathToPropertyName(*this);
    }

    return m_NodeInfos[index];
  };

  const PropertyKeyPath::NodeInfo &PropertyKeyPath::GetFirstNode() const
  {
    if (m_NodeInfos.empty())
    {
      mitkThrowException(InvalidPathNodeException) << "Error. Cannot return first path node. Path is empty.";
    }
    return this->GetNode(0);
  };

  PropertyKeyPath::NodeInfo &PropertyKeyPath::GetFirstNode()
  {
    if (m_NodeInfos.empty())
    {
      mitkThrowException(InvalidPathNodeException) << "Error. Cannot return first path node. Path is empty.";
    }
    return this->GetNode(0);
  };

  const PropertyKeyPath::NodeInfo &PropertyKeyPath::GetLastNode() const
  {
    if (m_NodeInfos.empty())
    {
      mitkThrowException(InvalidPathNodeException) << "Error. Cannot return last path node. Path is empty.";
    }
    return this->GetNode(GetSize() - 1);
  };

  PropertyKeyPath::NodeInfo &PropertyKeyPath::GetLastNode()
  {
    if (m_NodeInfos.empty())
    {
      mitkThrowException(InvalidPathNodeException) << "Error. Cannot return last path node. Path is empty.";
    }
    return this->GetNode(GetSize() - 1);
  };

  const PropertyKeyPath::NodeInfoVectorType &PropertyKeyPath::GetNodes() const { return m_NodeInfos; };

  bool PropertyKeyPath::operator==(const PropertyKeyPath &path) const { return m_NodeInfos == path.m_NodeInfos; };

  bool PropertyKeyPath::operator<(const PropertyKeyPath &right) const
  {
    auto rightIter = right.m_NodeInfos.cbegin();
    const auto rightEnd = right.m_NodeInfos.cend();
    for (const auto &leftPos : m_NodeInfos)
    {
      if (rightIter == rightEnd)
      {
        return false;
      }

      if (leftPos.name < rightIter->name)
      {
        return true;
      }
      if (rightIter->name < leftPos.name)
      {
        return false;
      }

      if (leftPos.type < rightIter->type)
      {
        return true;
      }
      if (rightIter->type < leftPos.type)
      {
        return false;
      }

      if (leftPos.selection < rightIter->selection)
      {
        return true;
      }
      if (rightIter->selection < leftPos.selection)
      {
        return false;
      }

      ++rightIter;
    }
    return rightIter != rightEnd;
  }

  bool PropertyKeyPath::operator>(const PropertyKeyPath &right) const
  {
    auto rightIter = right.m_NodeInfos.cbegin();
    const auto rightEnd = right.m_NodeInfos.cend();
    for (const auto &leftPos : m_NodeInfos)
    {
      if (rightIter == rightEnd)
        return false;

      if (leftPos.name > rightIter->name)
        return true;
      if (rightIter->name > leftPos.name)
        return false;

      if (leftPos.type > rightIter->type)
        return true;
      if (rightIter->type > leftPos.type)
        return false;

      if (leftPos.selection > rightIter->selection)
        return true;
      if (rightIter->selection > leftPos.selection)
        return false;
      ++rightIter;
    }
    return rightIter != rightEnd;
  }

  bool PropertyKeyPath::operator>=(const PropertyKeyPath &right) const { return !(*this < right); }

  bool PropertyKeyPath::operator<=(const PropertyKeyPath &right) const { return !(*this > right); }

  bool PropertyKeyPath::Equals(const PropertyKeyPath &path) const { return PropertyKeyPathsMatch(*this, path); };

  PropertyKeyPath &PropertyKeyPath::operator=(const PropertyKeyPath &path)
  {
    if (this != &path)
    {
      m_NodeInfos = path.m_NodeInfos;
    }

    return *this;
  };

  PropertyKeyPath &PropertyKeyPath::AddAnyElement()
  {
    m_NodeInfos.emplace_back("", NodeInfo::NodeType::AnyElement);
    return *this;
  };

  PropertyKeyPath &PropertyKeyPath::AddElement(const ElementNameType &name)
  {
    m_NodeInfos.emplace_back(name, NodeInfo::NodeType::Element);
    return *this;
  };

  PropertyKeyPath &PropertyKeyPath::AddAnySelection(const ElementNameType &name)
  {
    m_NodeInfos.emplace_back(name, NodeInfo::NodeType::AnySelection);
    return *this;
  };

  PropertyKeyPath &PropertyKeyPath::AddSelection(const ElementNameType &name, ItemSelectionIndex index)
  {
    m_NodeInfos.emplace_back(name, NodeInfo::NodeType::ElementSelection, index);
    return *this;
  };

  PropertyKeyPath::PropertyKeyPath() { this->Reset(); };

  PropertyKeyPath::PropertyKeyPath(const PropertyKeyPath &path) { *this = path; };

  PropertyKeyPath::~PropertyKeyPath(){};

  void PropertyKeyPath::Reset() { m_NodeInfos.clear(); };

  bool PropertyKeyPath::PropertyKeyPathsMatch(const PropertyKeyPath &left, const PropertyKeyPath &right)
  {
    auto leftPos = left.GetNodes().cbegin();
    auto rightPos = right.GetNodes().cbegin();
    auto leftEnd = left.GetNodes().cend();
    auto rightEnd = right.GetNodes().cend();

    while (leftPos != leftEnd && rightPos != rightEnd)
    {
      if (!leftPos->Matches(*rightPos))
      {
        break;
      }
      ++leftPos;
      ++rightPos;
    }

    if (leftPos == leftEnd && rightPos == rightEnd)
    {
      return true;
    }
    else
    {
      return false;
    }
  };

  std::ostream &operator<<(std::ostream &os, const PropertyKeyPath &value)
  {
    os << PropertyKeyPathToPropertyName(value);
    return os;
  };

  std::string PropertyKeyPathToPropertyRegEx(const PropertyKeyPath &tagPath)
  {
    std::ostringstream nameStream;

    PropertyKeyPath::PathIndexType i = 0;

    for (const auto &node : tagPath.GetNodes())
    {
      if (i)
      {
        nameStream << "\\.";
      }
      ++i;

      if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "([a-zA-Z0-9- ]+)";
      }
      else if (node.type != PropertyKeyPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << node.name;

        if (node.type == PropertyKeyPath::NodeInfo::NodeType::ElementSelection)
        {
          nameStream << "\\.\\[" << node.selection << "\\]";
        }
        else if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnySelection)
        {
          nameStream << "\\.\\[(\\d*)\\]";
        }
      }
      else
      {
        nameStream << "INVALIDNODE";
      }
    }

    return nameStream.str();
  };

  std::string PropertyKeyPathToPersistenceKeyRegEx(const PropertyKeyPath &tagPath)
  {
    std::ostringstream nameStream;

    PropertyKeyPath::PathIndexType i = 0;

    for (const auto &node : tagPath.GetNodes())
    {
      if (i)
      {
        nameStream << "_";
      }
      ++i;

      if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "([a-zA-Z0-9- ]+)";
      }
      else if (node.type != PropertyKeyPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << node.name;

        if (node.type == PropertyKeyPath::NodeInfo::NodeType::ElementSelection)
        {
          nameStream << "_\\[" << node.selection << "\\]";
        }
        else if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnySelection)
        {
          nameStream << "_\\[(\\d*)\\]";
        }
      }
      else
      {
        nameStream << "INVALIDNODE";
      }
    }

    return nameStream.str();
  };

  std::string PropertyKeyPathToPersistenceKeyTemplate(const PropertyKeyPath &tagPath)
  {
    std::ostringstream nameStream;

    int captureGroup = 1;

    PropertyKeyPath::PathIndexType i = 0;

    for (const auto &node : tagPath.GetNodes())
    {
      if (i)
      {
        nameStream << "_";
      }
      ++i;

      if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "$" << captureGroup++;
      }
      else if (node.type != PropertyKeyPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << node.name;

        if (node.type == PropertyKeyPath::NodeInfo::NodeType::ElementSelection)
        {
          nameStream << "_[" << node.selection << "]";
        }
        else if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnySelection)
        {
          nameStream << "_[$" << captureGroup++ << "]";
        }
      }
      else
      {
        nameStream << "INVALID_NODE";
      }
    }

    return nameStream.str();
  };

  std::string PropertyKeyPathToPersistenceNameTemplate(const PropertyKeyPath &tagPath)
  {
    std::ostringstream nameStream;

    int captureGroup = 1;

    PropertyKeyPath::PathIndexType i = 0;

    for (const auto &node : tagPath.GetNodes())
    {
      if (i)
      {
        nameStream << ".";
      }
      ++i;

      if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "$" << captureGroup++;
      }
      else if (node.type != PropertyKeyPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << node.name;

        if (node.type == PropertyKeyPath::NodeInfo::NodeType::ElementSelection)
        {
          nameStream << ".[" << node.selection << "]";
        }
        else if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnySelection)
        {
          nameStream << ".[$" << captureGroup++ << "]";
        }
      }
      else
      {
        nameStream << "INVALID_NODE";
      }
    }

    return nameStream.str();
  };

  PropertyKeyPath PropertyNameToPropertyKeyPath(const std::string &propertyName)
  {
    PropertyKeyPath result;

    std::regex reg_element("([a-zA-Z0-9- ]+)");
    std::regex reg_anySelection("\\[\\*\\]");
    std::regex reg_Selection("\\[(\\d+)\\]");

    std::istringstream f(propertyName);
    std::string subStr;

    PropertyKeyPath::ElementNameType name = "";

    while (getline(f, subStr, '.'))
    {
      if (subStr == "*")
      {
        if (!name.empty())
        {
          result.AddElement(name);
          name.clear();
        }

        result.AddAnyElement();
      }
      else
      {
        std::smatch sm;
        if (std::regex_match(subStr, sm, reg_anySelection))
        {
          if (!name.empty())
          {
            result.AddAnySelection(name);
            name.clear();
          }
          else
          { // invalid path
            return PropertyKeyPath();
          }
        }
        else if (std::regex_match(subStr, sm, reg_Selection))
        {
          if (!name.empty())
          {
            result.AddSelection(name, std::stoi(sm[1]));
            name.clear();
          }
          else
          { // invalid path
            return PropertyKeyPath();
          }
        }
        else if (std::regex_match(subStr, sm, reg_element))
        {
          if (!name.empty())
          { // store the last element and start the next
            result.AddElement(name);
          }
          name = sm[1];
        }
        else
        {
          return PropertyKeyPath();
        }
      }
    }

    if (!name.empty())
    { // add last element
      result.AddElement(name);
    }

    return result;
  };

  std::string PropertyKeyPathToPropertyName(const mitk::PropertyKeyPath &tagPath)
  {
    std::ostringstream nameStream;

    PropertyKeyPath::PathIndexType i = 0;

    for (const auto &node : tagPath.GetNodes())
    {
      if (i)
      {
        nameStream << ".";
      }
      ++i;

      if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "*";
      }
      else if (node.type != PropertyKeyPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << node.name;

        if (node.type == PropertyKeyPath::NodeInfo::NodeType::ElementSelection)
        {
          nameStream << ".[" << node.selection << "]";
        }
        else if (node.type == PropertyKeyPath::NodeInfo::NodeType::AnySelection)
        {
          nameStream << ".[*]";
        }
      }
      else
      {
        nameStream << "INVALID_NODE";
      }
    }

    return nameStream.str();
  };
} // namespace mitk
