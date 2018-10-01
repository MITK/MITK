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

#ifndef MITKRESTUTIL_H
#define MITKRESTUTIL_H

#include <string>
#include <vector>
#include "MitkCppRestSdkExports.h"

// hm.. maybe go after that warning at some time? seems like a nasty hack, but works so far :)
#pragma warning(disable : 4251)

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT RESTUtil
  {

  public:
    RESTUtil();
    virtual ~RESTUtil();

    std::string body_content()
    {
      return m_BodyContent;
    }

    std::string boundary()
    {
      return m_Boundary;
    }

    void ContentType(const std::string &contentType)
    {
      m_ContentType = contentType;
    }

    void AddParameter(const std::string &name, const std::string &value)
    {
      m_Params.push_back(std::move(std::pair<std::string, std::string>(name, value)));
    }

    void AddFile(const std::string &name, const std::string &value)
    {
      m_Files.push_back(std::move(std::pair<std::string, std::string>(name, value)));
    }

    std::string& GenBodyContent();

  private:
    std::string m_BoundaryPrefix;
    std::string m_RandChars;
    std::string m_Boundary = "boundary";
    std::string m_BodyContent;
    std::string m_ContentType;
    std::vector<std::pair<std::string, std::string>> m_Params;
    std::vector<std::pair<std::string, std::string>> m_Files;
  };
};

#endif // MITKRESTUTIL_H
