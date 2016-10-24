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

#include <mitkServiceInterface.h>

#include <string>
#include <vector>

namespace mitk
{
  class MimeType;

  /**
   * @ingroup IO
   * @ingroup MicroServices_Interfaces
   *
   * @brief The CustomMimeType class represents a custom mime-type which
   *        may be registered as a service object. It should only be used for mime-type registration,
   *        see also mitk::MimeType.
   *
   * Instances of this class are usually created and registered as a service.
   * They act as meta data information to allow the linking of files to reader and writer.
   * They write files to specific IFileReader instances and provide data format
   * meta-data for selecting compatible IFileWriter instances.
   * mirk::CustomMimetype should only be used to register mime-types. All other interaction should happen trough
   * mitk::MimeTypeProvider, from which registered mimetypes can be pulled. mitk::MimeType provides a safe and
   * memory-managed
   * way of interacting with Mimetypes.
   */
  class MITKCORE_EXPORT CustomMimeType
  {
  public:
    CustomMimeType();
    CustomMimeType(const std::string &name);
    CustomMimeType(const CustomMimeType &other);
    explicit CustomMimeType(const MimeType &other);

    virtual ~CustomMimeType();

    CustomMimeType &operator=(const CustomMimeType &other);
    CustomMimeType &operator=(const MimeType &other);

    /**
    * \brief Returns the unique name for the MimeType.
    */
    std::string GetName() const;

    /**
    * \brief Returns the human-readable Category of the mime-type. Allows grouping of similar mime-types (like Surfaces)
    */
    std::string GetCategory() const;

    /**
    * \brief Returns all extensions that this MimeType can handle.
    */
    std::vector<std::string> GetExtensions() const;

    /**
    * \brief Returns the Human readable comment of the MimeType, a string that describes its unique role.
    */
    std::string GetComment() const;

    /**
    * \brief Checks if the MimeType can handle file at the given location.
    *
    * In its base implementation, this function exclusively looks a the given string.
    * However, child classes can override this behaviour and peek into the file.
    */
    virtual bool AppliesTo(const std::string &path) const;

    /**
    * \brief Checks if the MimeType can handle the etension of the given path
    *
    * This function exclusively looks a the given string
    */
    bool MatchesExtension(const std::string &path) const;

    /**
    * \brief Provides the first matching extension
    *
    * Checks whether any of its extensions are present at the end of the provided path.
    * Returns the first found one.
    */
    std::string GetExtension(const std::string &path) const;

    /**
    * \brief Provides the filename minus the extension
    *
    * Checks whether any of its extensions are present at the end of the provided path.
    * Returns the filename without that extension and without the directory.
    */
    std::string GetFilenameWithoutExtension(const std::string &path) const;

    void SetName(const std::string &name);
    void SetCategory(const std::string &category);
    void SetExtension(const std::string &extension);
    void AddExtension(const std::string &extension);
    void SetComment(const std::string &comment);

    void Swap(CustomMimeType &r);

    virtual CustomMimeType *Clone() const;

  private:
    // returns true if an extension was found
    bool ParsePathForExtension(const std::string &path, std::string &extension, std::string &filename) const;

    struct Impl;
    Impl *d;
  };

  void swap(CustomMimeType &l, CustomMimeType &r);
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::CustomMimeType, "org.mitk.CustomMimeType")

#endif // MITKCUSTOMMIMETYPE_H
