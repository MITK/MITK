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

#ifndef ConfigFileReader_h
#define ConfigFileReader_h

#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <vector>

class ConfigFileReader

{
protected:

  typedef std::map<std::string, std::string> ContentType;
  typedef std::map<std::string, std::vector<std::string> > ListContentType;

  ContentType     m_ConfigContent;
  ListContentType m_ListContent;
  std::map<std::string, unsigned int > m_ListSize;

  std::string Trim(std::string const& source, char const * delim = " \t\r\n")
  {
    std::string result(source);
    std::string::size_type index = result.find_last_not_of(delim);
    if(index != std::string::npos)
      result.erase(++index);
    index = result.find_first_not_of(delim);
    if(index != std::string::npos)
      result.erase(0, index);
    else
      result.erase();
    return result;
  }

  std::string RemoveComment(std::string const& source, char const * delim = "#;")
  {
    std::string result(source);
    std::string::size_type index = result.find_first_of(delim);
    if(index != std::string::npos)
      result.erase(++index);
    return Trim(result);
  }

  std::string ListIndex(std::string const& section, unsigned int index) const
  {
    std::stringstream stream;
    stream << section << "/" << index;
    std::string result = stream.str();
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

  std::string ContentIndex(std::string const& section, std::string const& entry) const

  {
    std::string result = section + '/' + entry;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

  std::string ListSizeIndex(std::string const& section) const
  {
    std::string result = section;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

public:

  ConfigFileReader(std::string const& configFile)
  {
    ReadFile (configFile);
  }

  void ReadFile(std::string const& filePath)
  {
    std::ifstream file(filePath.c_str());
    ReadStream(file);
    file.close();
  }

  void ReadStream (std::istream& stream)
  {
    std::string line;
    std::string name;
    std::string value;
    std::string inSection;
    bool inConfigSection = true;
    int posEqual;
    while (std::getline(stream,line)) {
      line = RemoveComment(line, "#");

      if (! line.length()) continue;

      if (line[0] == '[') {
        inConfigSection = true;
        inSection = Trim(line.substr(1,line.find(']')-1));
        continue;
      }

      if(line[0] == '{') {
        std::string address = Trim(line.substr(1,line.find('}')-1));
        inSection = ListIndex(address, ListSize(address,0));
        m_ListSize[ListSizeIndex(address)]++;
        inConfigSection = false;
        continue;
      }

      if (inConfigSection)
      {
        posEqual=line.find('=');
        name  = Trim(line.substr(0,posEqual));
        value = Trim(line.substr(posEqual+1));
        m_ConfigContent[ContentIndex(inSection, name)]=value;
      }
      else
      {
        m_ListContent[inSection].push_back(Trim(line));
      }
    }
  }

  std::string Value(std::string const& section, std::string const& entry) const
  {
    std::string index = ContentIndex(section,entry);
    if (m_ConfigContent.find(index) == m_ConfigContent.end())
      throw "Entry doesn't exist " + section + entry;
    return m_ConfigContent.find(index)->second;
  }

  std::string Value(const std::string & section, const std::string & entry, const std::string& standard)
  {
    try {
      return Value(section, entry);
    }
    catch (const char *) {
      m_ConfigContent[ContentIndex(section, entry)] = standard;
      return standard;
    }
  }

  int IntValue(const std::string & section, const std::string & entry) const
  {
    int result;
    std::stringstream stream (Value(section, entry));
    stream >> result;
    return result;
  }

  int IntValue(const std::string & section, const std::string & entry, int standard)
  {
    try {
      return IntValue(section, entry);
    }
    catch (const char *) {
      std::stringstream stream;
      stream << standard;
      m_ConfigContent[ContentIndex(section, entry)] = stream.str();
      return standard;
    }
  }

  std::vector<std::string> Vector(std::string const& section, unsigned int index) const
  {
    if (m_ListContent.find(ListIndex(section, index)) == m_ListContent.end())
      throw "Entry doesn't exist " + section;
    return m_ListContent.find(ListIndex(section,index))->second;
  }

  unsigned int ListSize(std::string const& section) const
  {
    if (m_ListSize.find(ListSizeIndex(section)) == m_ListSize.end())
      throw "Entry doesn't exist " + section;
    return m_ListSize.find(ListSizeIndex(section))->second;
  }

  unsigned int ListSize(std::string const& section, unsigned int standard)
  {
    try {
      return ListSize(ListSizeIndex(section));
    }
    catch (...) {
      m_ListSize[ListSizeIndex(section)] = standard;
      return standard;
    }
  }
};

#endif