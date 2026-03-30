/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConfigFileReader_h
#define mitkConfigFileReader_h

#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>

/**
 * \brief Simple INI-style configuration file reader.
 *
 * Reads configuration files that contain sections enclosed in square brackets,
 * key=value pairs within those sections, and list blocks enclosed in curly braces.
 * Comments beginning with '#' are stripped. Lookups are case-insensitive.
 *
 * Example file format:
 * \code
 * [SectionName]
 * key = value
 *
 * {ListName}
 * item1
 * item2
 * \endcode
 *
 * \sa ConfigurationHolder
 */
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

  /**
   * \brief Construct a ConfigFileReader and immediately parse the given file.
   *
   * \param[in] configFile Path to the configuration file to read.
   */
  ConfigFileReader(std::string const& configFile)
  {
    ReadFile (configFile);
  }

  /**
   * \brief Read and parse a configuration file from disk.
   *
   * \param[in] filePath Path to the configuration file.
   */
  void ReadFile(std::string const& filePath)
  {
    std::ifstream file(filePath.c_str());
    ReadStream(file);
    file.close();
  }

  /**
   * \brief Parse configuration data from an input stream.
   *
   * Processes sections ([...]), list blocks ({...}), and key=value pairs.
   * Comments starting with '#' are removed.
   *
   * \param[in] stream The input stream to read from.
   */
  void ReadStream (std::istream& stream)
  {
    std::string line;
    std::string name;
    std::string value;
    std::string inSection;
    bool inConfigSection = true;
    std::string::size_type posEqual;
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

  /**
   * \brief Retrieve a string value from the configuration.
   *
   * \param[in] section The section name.
   * \param[in] entry The key name within the section.
   * \return The value associated with section::entry.
   * \throw std::string If the entry does not exist.
   */
  std::string Value(std::string const& section, std::string const& entry) const
  {
    std::string index = ContentIndex(section,entry);
    if (m_ConfigContent.find(index) == m_ConfigContent.end())
      throw std::string("Entry doesn't exist " + section +"::"+ entry);
    std::cout << section << "::" << entry << m_ConfigContent.find(index)->second << std::endl;
    return m_ConfigContent.find(index)->second;
  }

  /**
   * \brief Retrieve a string value, returning a default if the entry does not exist.
   *
   * \param[in] section The section name.
   * \param[in] entry The key name within the section.
   * \param[in] standard Default value to return and store if the entry is missing.
   * \return The value associated with section::entry, or the default.
   */
  std::string Value(const std::string & section, const std::string & entry, const std::string& standard)
  {
    try {
      return Value(section, entry);
    }
    catch (const std::string) {
      m_ConfigContent[ContentIndex(section, entry)] = standard;
      std::cout << section << "::" << entry << standard << " (default)" << std::endl;
      return standard;
    }
  }

  /**
   * \brief Retrieve an integer value from the configuration.
   *
   * \param[in] section The section name.
   * \param[in] entry The key name within the section.
   * \return The integer value parsed from the entry.
   * \throw std::string If the entry does not exist.
   */
  int IntValue(const std::string & section, const std::string & entry) const
  {
    int result;
    std::stringstream stream (Value(section, entry));
    stream >> result;
    return result;
  }

  /**
   * \brief Retrieve an integer value, returning a default if the entry does not exist.
   *
   * \param[in] section The section name.
   * \param[in] entry The key name within the section.
   * \param[in] standard Default value to return and store if the entry is missing.
   * \return The integer value, or the default.
   */
  int IntValue(const std::string & section, const std::string & entry, int standard)
  {
    try {
      return IntValue(section, entry);
    }
    catch (const std::string) {
      std::stringstream stream;
      stream << standard;
      m_ConfigContent[ContentIndex(section, entry)] = stream.str();
      std::cout << section << "::" << entry << stream.str() << "(default)" << std::endl;
      return standard;
    }
  }

  /**
   * \brief Retrieve a list block's content lines by section name and index.
   *
   * \param[in] section The list section name.
   * \param[in] index Zero-based index of the list block (multiple blocks with the same name are indexed sequentially).
   * \return A vector of strings, one per line in the list block.
   * \throw std::string If the entry does not exist.
   */
  std::vector<std::string> Vector(std::string const& section, unsigned int index) const
  {
    if (m_ListContent.find(ListIndex(section, index)) == m_ListContent.end())
    {
      throw std::string("Entry doesn't exist " + section);
    }
    return m_ListContent.find(ListIndex(section,index))->second;
  }

  /**
   * \brief Get the number of list blocks with the given section name.
   *
   * \param[in] section The list section name.
   * \return The number of list blocks.
   * \throw std::string If no list blocks with the given section exist.
   */
  unsigned int ListSize(std::string const& section) const
  {
    if (m_ListSize.find(ListSizeIndex(section)) == m_ListSize.end())
    {
      throw std::string("Entry doesn't exist " + section);
    }
    return m_ListSize.find(ListSizeIndex(section))->second;
  }

  /**
   * \brief Get the number of list blocks, returning a default if none exist.
   *
   * \param[in] section The list section name.
   * \param[in] standard Default count to return and store if no blocks exist.
   * \return The number of list blocks, or the default.
   */
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
