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

#include <mitkDICOMTagPath.h>
#include <mitkExceptionMacro.h>

#include <regex>

namespace
{
  std::string GenerateRegExForNumber(unsigned int tagNumber)
  {
    std::ostringstream resultRegEx;

    std::ostringstream hexNumber;
    hexNumber << std::hex << tagNumber; // default std::hex output is lowercase
    std::regex reg_character("([a-f]+)"); // so only check for lowercase characters here
    if (std::regex_search(hexNumber.str(), reg_character))
    {
      // hexNumber contains a characters from a-f
      // needs to be generated with lowercase and uppercase characters
      resultRegEx << "("
        << std::setw(4) << std::setfill('0') << std::hex << tagNumber
        << "|"
        << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << tagNumber << std::nouppercase
        << ")";
    }
    else
    {
      // only decimal values (0-9) contained in hexNumber - no need to modify the result
      resultRegEx << std::setw(4) << std::setfill('0') << std::hex << tagNumber;
    }

    return resultRegEx.str();
  }
}

namespace mitk
{

  DICOMTagPath::NodeInfo::
    NodeInfo() : type(NodeType::Invalid), tag(0, 0), selection(0)
  {
    ;
  };

  DICOMTagPath::NodeInfo::
    NodeInfo(const DICOMTag& aTag, NodeType aType, ItemSelectionIndex index) : type(aType), tag(aTag), selection(index)
  {};

  bool DICOMTagPath::NodeInfo::operator == (const NodeInfo& right) const
  {
    if (!(this->tag == right.tag)) return false;
    if (this->type != right.type) return false;
    if (this->selection != right.selection) return false;

    return true;
  };

  bool DICOMTagPath::NodeInfo::
    Matches(const NodeInfo& right) const
  {
    if (type == NodeType::AnyElement || right.type == NodeType::AnyElement)
    {
      return true;
    }
    else if (tag == right.tag && type != NodeType::Invalid && right.type != NodeType::Invalid)
    {
      if (type == NodeType::Element && right.type == NodeType::Element)
      {
        return true;
      }
      else if(selection == right.selection || type == NodeType::AnySelection || right.type == NodeType::AnySelection)
      {
        return true;
      }
    }
    return false;
  };

  bool DICOMTagPath::IsEmpty() const
  {
    return m_NodeInfos.empty();
  };

  bool
    DICOMTagPath::
    IsExplicit() const
  {
    for (const auto & pos : m_NodeInfos)
    {
      if ((pos.type == NodeInfo::NodeType::AnySelection) || (pos.type == NodeInfo::NodeType::AnyElement)) return false;
    }

    return true;
  };

  bool
    DICOMTagPath::
    HasItemSelectionWildcardsOnly() const
  {
    bool result = false;
    for (const auto & pos : m_NodeInfos)
    {
      if (pos.type == NodeInfo::NodeType::AnyElement) return false;
      result = result || pos.type == NodeInfo::NodeType::AnySelection;
    }

    return result;
  };

  DICOMTagPath::PathIndexType DICOMTagPath::Size() const
  {
    return m_NodeInfos.size();
  }

  DICOMTagPath::PathIndexType
    DICOMTagPath::
    AddNode(const NodeInfo& newNode)
  {
    m_NodeInfos.push_back(newNode);
    return m_NodeInfos.size() - 1;
  };

  const DICOMTagPath::NodeInfo&
    DICOMTagPath::
    GetNode(const PathIndexType& index) const
  {
    if (index >= Size())
    {
      mitkThrow() << "Error. Cannot return info of path node. Node index is out of bound. Index: " << index << "; Path: " << this->ToStr();
    }

    return m_NodeInfos[index];
  };

  DICOMTagPath::NodeInfo&
    DICOMTagPath::
    GetNode(const PathIndexType& index)
  {
    if (index >= Size())
    {
      mitkThrow() << "Error. Cannot return info of path node. Node index is out of bound. Index: " << index << "; Path: " << this->ToStr();
    }

    return m_NodeInfos[index];
  };

  const DICOMTagPath::NodeInfo&
    DICOMTagPath::
    GetFirstNode() const
  {
    return GetNode(0);
  };

  const DICOMTagPath::NodeInfo&
    DICOMTagPath::
    GetLastNode() const
  {
    return GetNode(Size() - 1);
  };

  DICOMTagPath::NodeInfo&
    DICOMTagPath::
    GetLastNode()
  {
    return GetNode(Size() - 1);
  };

  const DICOMTagPath::NodeInfoVectorType&
    DICOMTagPath::
    GetNodes() const
  {
    return m_NodeInfos;
  };

  std::string
    DICOMTagPath::
    ToStr() const
  {
    std::ostringstream nameStream;

    if (this->Size() == 0) return nameStream.str();

    PathIndexType i = 0;
    for (const auto& node : m_NodeInfos)
    {
      if (i)
      {
        nameStream << ".";
      }
      ++i;

      if (node.type == NodeInfo::NodeType::AnyElement)
      {
        nameStream << "*";
      }
      else if (node.type != NodeInfo::NodeType::Invalid)
      {
        nameStream << "("
                   << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetGroup() << std::nouppercase
                   << ","
                   << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetElement() << std::nouppercase
                   << ")";

        if (node.type == NodeInfo::NodeType::SequenceSelection)
        {
          nameStream << "[" << node.selection << "]";
        }
        else if (node.type == NodeInfo::NodeType::AnySelection)
        {
          nameStream << "[*]";
        }
      }
      else
      {
        nameStream << "INVALID_NODE";
      }
    }

    return nameStream.str();
  };

  bool
    DICOMTagPath::
    operator == (const DICOMTagPath& path) const
  {
    return this->m_NodeInfos == path.m_NodeInfos;
  };

  bool
    DICOMTagPath::
    operator < (const DICOMTagPath& right) const
  {
    auto rightIter = right.m_NodeInfos.cbegin();
    const auto rightEnd = right.m_NodeInfos.cend();
    for (const auto& leftPos : m_NodeInfos)
    {
      if (rightIter == rightEnd) return false;

      if (leftPos.tag.GetElement() < rightIter->tag.GetElement()) return true;
      if (rightIter->tag.GetElement() < leftPos.tag.GetElement()) return false;

      if (leftPos.tag.GetGroup() < rightIter->tag.GetGroup()) return true;
      if (rightIter->tag.GetGroup()< leftPos.tag.GetGroup()) return false;

      if (leftPos.type < rightIter->type) return true;
      if (rightIter->type< leftPos.type) return false;

      if (leftPos.selection < rightIter->selection) return true;
      if (rightIter->selection< leftPos.selection) return false;
      ++rightIter;
    }
    return rightIter != rightEnd;
  }

  bool
    DICOMTagPath::
    Equals(const DICOMTagPath& path) const
  {
    return DICOMTagPathesMatch(*this, path);
  };

  DICOMTagPath&
    DICOMTagPath::
    operator = (const DICOMTagPath& path)
  {
    if (&path == this) return *this;

    this->m_NodeInfos = path.m_NodeInfos;

    return *this;
  };

  DICOMTagPath&
    DICOMTagPath::AddAnyElement()
  {
    m_NodeInfos.emplace_back(DICOMTag(0,0), NodeInfo::NodeType::AnyElement);
    return *this;
  };

  DICOMTagPath&
    DICOMTagPath::AddElement(unsigned int group, unsigned int element)
  {
    m_NodeInfos.emplace_back(DICOMTag(group, element), NodeInfo::NodeType::Element);
    return *this;
  };

  DICOMTagPath&
    DICOMTagPath::AddAnySelection(unsigned int group, unsigned int element)
  {
    m_NodeInfos.emplace_back(DICOMTag(group, element), NodeInfo::NodeType::AnySelection);
    return *this;
  };

  DICOMTagPath&
    DICOMTagPath::AddSelection(unsigned int group, unsigned int element, ItemSelectionIndex index)
  {
    m_NodeInfos.emplace_back(DICOMTag(group, element), NodeInfo::NodeType::SequenceSelection, index);
    return *this;
  };

  DICOMTagPath&
    DICOMTagPath::
    FromStr(const std::string& pathStr)
  {
    NodeInfoVectorType result;
    std::istringstream f(pathStr);
    std::string subStr;

    while (getline(f, subStr, '.'))
    {
      NodeInfo info;

      if (subStr == "*")
      {
        info.type = NodeInfo::NodeType::AnyElement;
      }
      else
      {
        std::regex reg_element("\\(([A - Fa - f\\d]{4}),([A - Fa - f\\d]{4})\\)");
        std::regex reg_anySelection("\\(([A - Fa - f\\d]{4}),([A - Fa - f\\d]{4})\\)\\[\\*\\]");
        std::regex reg_Selection("\\(([A - Fa - f\\d]{4}),([A - Fa - f\\d]{4})\\)\\[(\\d+)\\]");
        std::smatch sm;
        if (std::regex_match(subStr, sm, reg_anySelection))
        {
          info.type = NodeInfo::NodeType::AnySelection;
          info.tag = DICOMTag(std::stoul(sm[1], nullptr, 16), std::stoul(sm[2], nullptr, 16));
        }
        else if (std::regex_match(subStr, sm, reg_Selection))
        {
          info.type = NodeInfo::NodeType::SequenceSelection;
          info.tag = DICOMTag(std::stoul(sm[1], nullptr, 16), std::stoul(sm[2], nullptr, 16));
          info.selection = std::stoi(sm[3]);
        }
        else if (std::regex_match(subStr, sm, reg_element))
        {
          info.type = NodeInfo::NodeType::Element;
          info.tag = DICOMTag(std::stoul(sm[1], nullptr, 16), std::stoul(sm[2], nullptr, 16));
        }
      }
      result.push_back(info);
    }

    this->m_NodeInfos.swap(result);
    return *this;
  };

  DICOMTagPath::DICOMTagPath()
  {
    Reset();
  };

  DICOMTagPath::
    DICOMTagPath(const DICOMTagPath& path)
  {
    *this = path;
  };

  DICOMTagPath::
    DICOMTagPath(const DICOMTag& tag)
  {
    m_NodeInfos.emplace_back(tag, NodeInfo::NodeType::Element);
  };

  DICOMTagPath::DICOMTagPath(unsigned int group, unsigned int element)
  {
    m_NodeInfos.emplace_back(DICOMTag(group,element));
  };

  DICOMTagPath::
    ~DICOMTagPath() {};

  void
    DICOMTagPath::
    Reset()
  {
    this->m_NodeInfos.clear();
  };

  bool
    DICOMTagPath::
    DICOMTagPathesMatch(const DICOMTagPath& left, const DICOMTagPath& right)
  {
    auto leftPos = left.GetNodes().cbegin();
    auto rightPos = right.GetNodes().cbegin();
    auto leftEnd = left.GetNodes().cend();
    auto rightEnd = right.GetNodes().cend();

    while (leftPos != leftEnd && rightPos != rightEnd)
    {
      if (!leftPos->Matches(*rightPos)) break;
      ++leftPos;
      ++rightPos;
    }

    if (leftPos == leftEnd && rightPos == rightEnd) return true;
    else return false;
  };

  std::ostream & operator<<(std::ostream &os, const DICOMTagPath &value)
  {
    os << value.ToStr();
    return os;
  };

  std::string DICOMTagPathToPropertyRegEx(const DICOMTagPath& tagPath)
  {
    std::ostringstream nameStream;

    nameStream << "DICOM";

    for (const auto& node : tagPath.GetNodes())
    {
      nameStream << "\\.";

      if (node.type == DICOMTagPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "([A-Fa-f\\d]{4})\\.([A-Fa-f\\d]{4})";
      }
      else if (node.type != DICOMTagPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << GenerateRegExForNumber(node.tag.GetGroup())
          << "\\."
          << GenerateRegExForNumber(node.tag.GetElement());

        if (node.type == DICOMTagPath::NodeInfo::NodeType::SequenceSelection)
        {
          nameStream << "\\.\\[" << node.selection << "\\]";
        }
        else if (node.type == DICOMTagPath::NodeInfo::NodeType::AnySelection)
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

  std::string DICOMTagPathToPersistenceKeyRegEx(const DICOMTagPath& tagPath)
  {
    std::ostringstream nameStream;

    nameStream << "DICOM";

    for (const auto& node : tagPath.GetNodes())
    {
      nameStream << "_";

      if (node.type == DICOMTagPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "([A-Fa-f\\d]{4})_([A-Fa-f\\d]{4})";
      }
      else if (node.type != DICOMTagPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << GenerateRegExForNumber(node.tag.GetGroup())
          << "_"
          << GenerateRegExForNumber(node.tag.GetElement());

        if (node.type == DICOMTagPath::NodeInfo::NodeType::SequenceSelection)
        {
          nameStream << "_\\[" << node.selection << "\\]";
        }
        else if (node.type == DICOMTagPath::NodeInfo::NodeType::AnySelection)
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

  std::string DICOMTagPathToPersistenceKeyTemplate(const DICOMTagPath& tagPath)
  {
    std::ostringstream nameStream;

    nameStream << "DICOM";

    int captureGroup = 1;

    for (const auto& node : tagPath.GetNodes())
    {
      nameStream << "_";

      if (node.type == DICOMTagPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "$" << captureGroup++;
        nameStream << "_$" << captureGroup++;
      }
      else if (node.type != DICOMTagPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetGroup() << std::nouppercase << "_"
          << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetElement();

        if (node.type == DICOMTagPath::NodeInfo::NodeType::SequenceSelection)
        {
          nameStream << "_[" << node.selection << "]";
        }
        else if (node.type == DICOMTagPath::NodeInfo::NodeType::AnySelection)
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

  std::string DICOMTagPathToPersistenceNameTemplate(const DICOMTagPath& tagPath)
  {
    std::ostringstream nameStream;

    nameStream << "DICOM";

    int captureGroup = 1;

    for (const auto& node : tagPath.GetNodes())
    {
      nameStream << ".";

      if (node.type == DICOMTagPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "$" << captureGroup++;
        nameStream << ".$" << captureGroup++;
      }
      else if (node.type != DICOMTagPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetGroup() << std::nouppercase << "."
          << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetElement();

        if (node.type == DICOMTagPath::NodeInfo::NodeType::SequenceSelection)
        {
          nameStream << ".[" << node.selection << "]";
        }
        else if (node.type == DICOMTagPath::NodeInfo::NodeType::AnySelection)
        {
          nameStream << ".[$"<<captureGroup++<<"]";
        }
      }
      else
      {
        nameStream << "INVALID_NODE";
      }
    }

    return nameStream.str();
  };


  std::string DICOMTagPathToDCMTKSearchPath(const DICOMTagPath& tagPath)
  {
    if (!tagPath.IsExplicit() && !tagPath.HasItemSelectionWildcardsOnly())
    {
      mitkThrow() << "Cannot convert DICOMTagPath into DCMTK search path. Path has element wild cards. Path: " << tagPath.ToStr();
    }

    return tagPath.ToStr();
  };


  DICOMTagPath
    PropertyNameToDICOMTagPath(const std::string& propertyName)
  {
    DICOMTagPath result;

    std::istringstream f(propertyName);
    std::string subStr;

    if (getline(f, subStr, '.'))
    {
      if (subStr != "DICOM")
      {
        return DICOMTagPath();
      }

      unsigned int nrCount = 0;
      unsigned long group = 0;
      unsigned long element = 0;

      while (getline(f, subStr, '.'))
      {
        if (subStr == "*")
        {
          if (nrCount == 2)
          { //add last element
            result.AddElement(group, element);
            nrCount = 0;
          }
          else if (nrCount != 0)
          { //invalid path
            return DICOMTagPath();
          }

          result.AddAnyElement();
        }
        else
        {
          std::regex reg_element("([A-Fa-f\\d]{4})");
          std::regex reg_anySelection("\\[\\*\\]");
          std::regex reg_Selection("\\[(\\d+)\\]");
          std::smatch sm;
          if (std::regex_match(subStr, sm, reg_anySelection))
          {
            if (nrCount == 2)
            {
              result.AddAnySelection(group, element);
              nrCount = 0;
            }
            else
            { //invalid path
              return DICOMTagPath();
            }
          }
          else if (std::regex_match(subStr, sm, reg_Selection))
          {
            if (nrCount == 2)
            {
              result.AddSelection(group, element, std::stoi(sm[1]));
              nrCount = 0;
            }
            else
            { //invalid path
              return DICOMTagPath();
            }
          }
          else if (std::regex_match(subStr, sm, reg_element))
          {
            if (nrCount == 0)
            {
              group = std::stoul(sm[1], nullptr, 16);
              ++nrCount;
            }
            else if (nrCount == 1)
            {
              element = std::stoul(sm[1], nullptr, 16);
              ++nrCount;
            }
            else if (nrCount == 2)
            { //store the last element and start the next
              result.AddElement(group, element);
              group = std::stoul(sm[1], nullptr, 16);
              nrCount = 1;
            }
            else
            { //invalid path
              return DICOMTagPath();
            }
          }
        }
      }

      if (nrCount == 2)
      { //add last element
        result.AddElement(group, element);
        nrCount = 0;
      }
    }

    return result;
  };

  std::string
    DICOMTagPathToPropertyName(const mitk::DICOMTagPath& tagPath)
  {
    std::ostringstream nameStream;

    nameStream << "DICOM";

    for (const auto& node : tagPath.GetNodes())
    {
      nameStream << ".";

      if (node.type == DICOMTagPath::NodeInfo::NodeType::AnyElement)
      {
        nameStream << "*";
      }
      else if (node.type != DICOMTagPath::NodeInfo::NodeType::Invalid)
      {
        nameStream << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetGroup() << std::nouppercase << "."
          << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << node.tag.GetElement();

        if (node.type == DICOMTagPath::NodeInfo::NodeType::SequenceSelection)
        {
          nameStream << ".[" << node.selection << "]";
        }
        else if (node.type == DICOMTagPath::NodeInfo::NodeType::AnySelection)
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
}