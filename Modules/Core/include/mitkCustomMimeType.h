/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCustomMimeType_h
#define mitkCustomMimeType_h

#include <MitkCoreExports.h>

#include <mitkServiceInterface.h>

#include <string>
#include <vector>

namespace mitk
{
  class MimeType;

  /**
   * \ingroup IO
   * \ingroup MicroServices_Interfaces
   *
   * \brief A custom mime-type that can be registered as a service object.
   *
   * Instances of this class are usually created and registered as a
   * CppMicroServices service. They act as meta-data that links files to
   * compatible IFileReader and IFileWriter instances by mapping file extensions
   * to unique mime-type names.
   *
   * CustomMimeType should only be used to define and register mime-types. For
   * all other interaction (querying, matching), use IMimeTypeProvider to retrieve
   * registered mime-types as mitk::MimeType objects, which provide a safe,
   * memory-managed, immutable view.
   *
   * \sa MimeType
   * \sa IMimeTypeProvider
   * \sa IOMimeTypes
   * \sa IFileReader
   * \sa IFileWriter
   */
  class MITKCORE_EXPORT CustomMimeType
  {
  public:
    /**
     * \brief Construct a default CustomMimeType with an empty name.
     */
    CustomMimeType();

    /**
     * \brief Construct a CustomMimeType with the given unique name.
     * \param[in] name The unique identifier for this mime-type
     *            (e.g. "application/vnd.mitk.image.nrrd").
     */
    CustomMimeType(const std::string &name);

    /**
     * \brief Copy constructor.
     * \param[in] other The CustomMimeType to copy from.
     */
    CustomMimeType(const CustomMimeType &other);

    /**
     * \brief Construct from an immutable MimeType wrapper.
     * \param[in] other The MimeType to copy properties from.
     */
    explicit CustomMimeType(const MimeType &other);

    virtual ~CustomMimeType();

    /**
     * \brief Copy assignment operator.
     * \param[in] other The CustomMimeType to assign from.
     * \return Reference to this object.
     */
    CustomMimeType &operator=(const CustomMimeType &other);

    /**
     * \brief Assignment from an immutable MimeType wrapper.
     * \param[in] other The MimeType to assign from.
     * \return Reference to this object.
     */
    CustomMimeType &operator=(const MimeType &other);

    /**
     * \brief Get the unique name for this mime-type.
     * \return The mime-type name string (e.g. "application/vnd.mitk.image.nrrd").
     */
    std::string GetName() const;

    /**
     * \brief Get the human-readable category of this mime-type.
     * \return The category string (e.g. "Images", "Surfaces"). Allows
     *         grouping of similar mime-types.
     */
    std::string GetCategory() const;

    /**
     * \brief Get all file extensions this mime-type can handle.
     * \return A vector of file extensions (without leading dots, e.g. "nrrd", "nhdr").
     */
    std::vector<std::string> GetExtensions() const;

    /**
     * \brief Get a human-readable description of this mime-type.
     * \return A comment string. If no explicit comment was set, a fallback is
     *         generated from the first extension (e.g. "nrrd File") or "Unknown".
     */
    std::string GetComment() const;

    /**
     * \brief Check whether this mime-type can handle the file at the given path.
     * \param[in] path The file path to check.
     * \return \c true if this mime-type applies to the given file.
     *
     * The base implementation checks only the file extension. Subclasses may
     * override this method to inspect file contents (e.g. DICOM magic bytes).
     */
    virtual bool AppliesTo(const std::string &path) const;

    /**
     * \brief Check whether the file extension of the given path matches this mime-type.
     * \param[in] path The file path to check.
     * \return \c true if the path ends with one of this mime-type's extensions.
     *
     * This method performs a case-insensitive extension comparison only;
     * it does not inspect file contents.
     */
    bool MatchesExtension(const std::string &path) const;

    /**
     * \brief Get the first matching extension found at the end of the given path.
     * \param[in] path The file path to inspect.
     * \return The matching extension (with leading dot), or an empty string if none matched.
     */
    std::string GetExtension(const std::string &path) const;

    /**
     * \brief Get the filename from the given path, without the matching extension.
     * \param[in] path The file path to inspect.
     * \return The filename without directory and without the matched extension,
     *         or an empty string if no extension matched.
     */
    std::string GetFilenameWithoutExtension(const std::string &path) const;

    /**
     * \brief Set the unique name of this mime-type.
     * \param[in] name The new mime-type name.
     */
    void SetName(const std::string &name);

    /**
     * \brief Set the category for grouping this mime-type.
     * \param[in] category The category string (e.g. "Images").
     */
    void SetCategory(const std::string &category);

    /**
     * \brief Set a single file extension, replacing all existing extensions.
     * \param[in] extension The file extension (without leading dot).
     */
    void SetExtension(const std::string &extension);

    /**
     * \brief Add a file extension to the list if not already present (case-insensitive).
     * \param[in] extension The file extension to add (without leading dot).
     */
    void AddExtension(const std::string &extension);

    /**
     * \brief Set the human-readable comment for this mime-type.
     * \param[in] comment The comment string.
     */
    void SetComment(const std::string &comment);

    /**
     * \brief Swap the contents of this CustomMimeType with another.
     * \param[in,out] r The CustomMimeType to swap with.
     */
    void Swap(CustomMimeType &r);

    /**
     * \brief Create a heap-allocated copy of this CustomMimeType.
     * \return A pointer to the cloned object. The caller takes ownership.
     */
    virtual CustomMimeType *Clone() const;

  private:
    // returns true if an extension was found
    bool ParsePathForExtension(const std::string &path, std::string &extension, std::string &filename) const;

    struct Impl;
    Impl *d;
  };

  /**
   * \brief Swap two CustomMimeType objects.
   * \param[in,out] l First CustomMimeType.
   * \param[in,out] r Second CustomMimeType.
   */
  void swap(CustomMimeType &l, CustomMimeType &r);
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::CustomMimeType, "org.mitk.CustomMimeType")

#endif
