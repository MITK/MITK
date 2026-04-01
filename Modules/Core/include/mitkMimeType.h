/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMimeType_h
#define mitkMimeType_h

#include <MitkCoreExports.h>

#include <usSharedData.h>

#include <vector>

namespace mitk
{
  class CustomMimeType;

  /**
   * \ingroup IO
   *
   * \brief Immutable, value-type wrapper for a registered mime-type.
   *
   * MimeType wraps a mitk::CustomMimeType with copy-on-write semantics,
   * providing a stack-friendly, memory-safe handle for consumers that need
   * to query mime-type information without managing raw pointers.
   *
   * To register a new mime-type, use the CustomMimeType class instead.
   * Wrapping into a MimeType is performed automatically by the framework.
   * In all other cases you should use mitk::MimeType when working with
   * mime-types.
   *
   * \sa CustomMimeType
   * \sa IMimeTypeProvider
   * \sa IOMimeTypes
   */
  class MITKCORE_EXPORT MimeType
  {
  public:
    /**
     * \brief Construct a default (invalid) MimeType.
     */
    MimeType();

    /**
     * \brief Copy constructor.
     * \param[in] other The MimeType to copy from.
     */
    MimeType(const MimeType &other);

    /**
     * \brief Construct a MimeType from a CustomMimeType with a service rank and id.
     * \param[in] x The CustomMimeType to wrap.
     * \param[in] rank The service ranking of this mime-type.
     * \param[in] id The service id of this mime-type.
     */
    MimeType(const CustomMimeType &x, int rank, long id);

    ~MimeType();

    /**
     * \brief Copy assignment operator.
     * \param[in] other The MimeType to assign from.
     * \return Reference to this object.
     */
    MimeType &operator=(const MimeType &other);

    /**
     * \brief Equality comparison based on the mime-type name.
     * \param[in] other The MimeType to compare with.
     * \return \c true if both MimeType objects have the same name.
     */
    bool operator==(const MimeType &other) const;

    /**
     * \brief Less-than comparison based on rank and service id.
     * \param[in] other The MimeType to compare with.
     * \return \c true if this MimeType is ranked lower than \c other.
     */
    bool operator<(const MimeType &other) const;

    /**
     * \brief Get the unique name of this mime-type.
     * \return The mime-type name (e.g. "application/vnd.mitk.image.nrrd").
     * \sa CustomMimeType::GetName()
     */
    std::string GetName() const;

    /**
     * \brief Get the human-readable category (e.g. "Images", "Surfaces").
     * \return The category string.
     * \sa CustomMimeType::GetCategory()
     */
    std::string GetCategory() const;

    /**
     * \brief Get the list of file extensions this mime-type handles.
     * \return A vector of file extensions (without leading dots).
     * \sa CustomMimeType::GetExtensions()
     */
    std::vector<std::string> GetExtensions() const;

    /**
     * \brief Get a human-readable comment describing this mime-type.
     * \return The comment string.
     * \sa CustomMimeType::GetComment()
     */
    std::string GetComment() const;

    /**
     * \brief Extract the filename without the matching extension from a path.
     * \param[in] path The file path to process.
     * \return The filename without the extension matched by this mime-type.
     * \sa CustomMimeType::GetFilenameWithoutExtension()
     */
    std::string GetFilenameWithoutExtension(const std::string &path) const;

    /**
     * \brief Check whether this mime-type can handle the given file path.
     * \param[in] path The file path to check.
     * \return \c true if this mime-type applies to the given path.
     * \sa CustomMimeType::AppliesTo()
     */
    bool AppliesTo(const std::string &path) const;

    /**
     * \brief Check whether the extension of the given path matches this mime-type.
     * \param[in] path The file path to check.
     * \return \c true if the path's extension matches one of this mime-type's extensions.
     * \sa CustomMimeType::MatchesExtension()
     */
    bool MatchesExtension(const std::string &path) const;

    /**
     * \brief Check whether this MimeType object is valid (has a non-empty name).
     * \return \c true if this MimeType has been properly initialized with a name.
     */
    bool IsValid() const;

    /**
     * \brief Swap the contents of this MimeType with another.
     * \param[in,out] m The MimeType to swap with.
     */
    void Swap(MimeType &m);

  private:
    struct Impl;

    // Use C++11 shared_ptr instead
    us::SharedDataPointer<const Impl> m_Data;
  };

  /**
   * \brief Swap two MimeType objects.
   * \param[in,out] m1 First MimeType.
   * \param[in,out] m2 Second MimeType.
   */
  MITKCORE_EXPORT void swap(MimeType &m1, MimeType &m2);

  /**
   * \brief Stream output operator for MimeType.
   * \param[in,out] os The output stream.
   * \param[in] mimeType The MimeType to print.
   * \return Reference to the output stream.
   */
  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &os, const MimeType &mimeType);
}

#endif
