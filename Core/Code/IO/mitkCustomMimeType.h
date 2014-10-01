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

#ifndef MITKCUSTOMMIMETYPE_H
#define MITKCUSTOMMIMETYPE_H

#include <MitkCoreExports.h>

#include <usServiceInterface.h>

#include <string>
#include <vector>

namespace mitk {

class MimeType;

class MITK_CORE_EXPORT CustomMimeType
{
public:

  CustomMimeType();
  CustomMimeType(const std::string& name);
  CustomMimeType(const CustomMimeType& other);
  explicit CustomMimeType(const MimeType& other);

  ~CustomMimeType();

  CustomMimeType& operator=(const CustomMimeType& other);
  CustomMimeType& operator=(const MimeType& other);

  std::string GetName() const;
  std::string GetCategory() const;
  std::vector<std::string> GetExtensions() const;
  std::string GetComment() const;

  void SetName(const std::string& name);
  void SetCategory(const std::string& category);
  void SetExtension(const std::string& extension);
  void AddExtension(const std::string& extension);
  void SetComment(const std::string& comment);

  void Swap(CustomMimeType& r);

private:

  struct Impl;
  Impl* d;
};

void swap(CustomMimeType& l, CustomMimeType& r);

}

US_DECLARE_SERVICE_INTERFACE(mitk::CustomMimeType, "org.mitk.CustomMimeType")

#endif // MITKCUSTOMMIMETYPE_H
