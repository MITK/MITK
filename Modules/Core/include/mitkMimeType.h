/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

    /** @see mitk::CustomMimeType::GetName()*/
    std::string GetName() const;

    /** @see mitk::CustomMimeType::GetCategory()*/
    std::string GetCategory() const;

    /** @see mitk::CustomMimeType::GetExtensions()*/
    std::vector<std::string> GetExtensions() const;

    /** @see mitk::CustomMimeType::GetComment()*/
    std::string GetComment() const;

    /** @see mitk::CustomMimeType::GetFileNameWithoutExtension()*/
    std::string GetFilenameWithoutExtension(const std::string &path) const;

    /** @see mitk::CustomMimeType::AppliesTo()*/
    bool AppliesTo(const std::string &path) const;

    /** @see mitk::CustomMimeType::MatchesExtension()*/
    bool MatchesExtension(const std::string &path) const;

    /** @see mitk::CustomMimeType::IsValid()*/
    bool IsValid() const;

    /** @see mitk::CustomMimeType::Swap()*/
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
