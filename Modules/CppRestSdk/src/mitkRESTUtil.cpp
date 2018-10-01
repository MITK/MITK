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

#include "mitkRESTUtil.h"

#include <iostream>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <future>

#include <filesystem>

#include <mitkCommon.h>

mitk::RESTUtil::RESTUtil() {}

mitk::RESTUtil::~RESTUtil() {}

std::string& mitk::RESTUtil::GenBodyContent()
{
  std::vector<std::future<std::string> > futures;
  m_BodyContent.clear();

  for (auto &file : m_Files)
  {
    std::future<std::string> content_futures = std::async(std::launch::async, [&file]()
    {
      std::ifstream ifile(file.second, std::ios::binary | std::ios::ate);
      std::streamsize size = ifile.tellg();
      ifile.seekg(0, std::ios::beg);
      char *buff = new char[size];
      ifile.read(buff, size);
      ifile.close();
      std::string ret(buff, size);
      delete[] buff;
      return ret;
    });
    futures.push_back(std::move(content_futures));
  }

  for (auto &param : m_Params)
  {
    m_BodyContent += "\r\n--";
    m_BodyContent += m_Boundary;
    m_BodyContent += "\r\nContent-Disposition: form-data; name=\"";
    m_BodyContent += param.first;
    m_BodyContent += "\"\r\n\r\n";
    m_BodyContent += param.second;
  }

  for (size_t i = 0; i < m_Files.size(); ++i)
  {
    std::string fileContent = futures[i].get();
    
    auto filename = std::experimental::filesystem::path(m_Files[i].second).filename();

    m_BodyContent += "\r\n--";
    m_BodyContent += m_Boundary;
    m_BodyContent += "\r\nContent-Disposition: form-data; name=\"";
    m_BodyContent += m_Files[i].first;
    m_BodyContent += "\"; filename=\"";
    m_BodyContent += filename.string();
    m_BodyContent += "\"\r\nContent-Type: ";
    m_BodyContent += m_ContentType;
    m_BodyContent += "\r\n\r\n";
    m_BodyContent += fileContent;
  }
  m_BodyContent += "\r\n--";
  m_BodyContent += m_Boundary;
  m_BodyContent += "--\r\n";
  return m_BodyContent;
}
