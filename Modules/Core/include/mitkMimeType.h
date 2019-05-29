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

namespace mitk
{
  class CustomMimeType;

  /**
   * @ingroup IO
   *
   * @brief The MimeType class represens a registered mime-type. It is an immutable wrapper for mitk::CustomMimeType
   * that makes memory handling easier by providing a stack-object for the user.
   *
   * If you want to register a new MimeType, use the CustomMimeType class instead. Wrapping will be performed for you
   * automatically.
   * In all other cases you should use mitk::MimeType when working with mime-types.
   */
  class MITKCORE_EXPORT MimeType
  {
  public:
    MimeType();
    MimeType(const MimeType &other);
    MimeType(const CustomMimeType &x, int rank, long id);

    ~MimeType();

    MimeType &operator=(const MimeType &other);
    bool operator==(const MimeType &other) const;

    bool operator<(const MimeType &other) const;

    /** @See mitk::CustomMimeType::GetName()*/
    std::string GetName() const;

    /** @See mitk::CustomMimeType::GetCategory()*/
    std::string GetCategory() const;

    /** @See mitk::CustomMimeType::GetExtensions()*/
    std::vector<std::string> GetExtensions() const;

    /** @See mitk::CustomMimeType::GetComment()*/
    std::string GetComment() const;

    /** @See mitk::CustomMimeType::GetFileNameWithoutExtension()*/
    std::string GetFilenameWithoutExtension(const std::string &path) const;

    /** @See mitk::CustomMimeType::AppliesTo()*/
    bool AppliesTo(const std::string &path) const;

    /** @See mitk::CustomMimeType::MatchesExtension()*/
    bool MatchesExtension(const std::string &path) const;

    /** @See mitk::CustomMimeType::IsValid()*/
    bool IsValid() const;

    /** @See mitk::CustomMimeType::Swap()*/
    void Swap(MimeType &m);

  private:
    struct Impl;

    // Use C++11 shared_ptr instead
    us::SharedDataPointer<const Impl> m_Data;
  };

  MITKCORE_EXPORT void swap(MimeType &m1, MimeType &m2);

  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &os, const MimeType &mimeType);
}

#endif // MITKMIMETYPE_H
