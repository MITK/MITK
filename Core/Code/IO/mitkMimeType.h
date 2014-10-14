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

#ifndef MITKMIMETYPE_H
#define MITKMIMETYPE_H

#include <MitkCoreExports.h>

#include <usSharedData.h>

#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace mitk {

class CustomMimeType;

class MITK_CORE_EXPORT MimeType
{

public:

  MimeType();
  MimeType(const MimeType& other);
  MimeType(const CustomMimeType& x, int rank, long id);

  ~MimeType();

  MimeType& operator=(const MimeType& other);
  bool operator==(const MimeType& other) const;

  bool operator<(const MimeType& other) const;

  std::string GetName() const;
  std::string GetCategory() const;
  std::vector<std::string> GetExtensions() const;
  std::string GetComment() const;

  bool AppliesTo(const std::string& path) const;

  bool IsValid() const;

  void Swap(MimeType& m);

private:

  struct Impl;

  // Use C++11 shared_ptr instead
  us::SharedDataPointer<const Impl> m_Data;
};

void swap(MimeType& m1, MimeType& m2);

std::ostream& operator<<(std::ostream& os, const MimeType& mimeType);

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // MITKMIMETYPE_H
