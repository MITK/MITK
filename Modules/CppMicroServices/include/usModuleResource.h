/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USMODULERESOURCE_H
#define USMODULERESOURCE_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <ostream>
#include <vector>

US_MSVC_PUSH_DISABLE_WARNING(4396)

namespace us {

class ModuleResourcePrivate;
class ModuleResourceContainer;

/**
 * \ingroup MicroServices
 *
 * \brief Represents a resource (text file, image, etc.) embedded in a CppMicroServices module.
 *
 * A \c %ModuleResource object provides information about a resource (external file) which
 * was embedded into this module's shared library. \c %ModuleResource objects can be obtained
 * be calling Module#GetResource or Module#FindResources.
 *
 * Example code for retrieving a resource object and reading its contents:
 * \snippet uServices-resources/main.cpp 1
 *
 * %ModuleResource objects have value semantics and copies are very inexpensive.
 *
 * \see ModuleResourceStream
 * \see \ref MicroServices_Resources
 */
class MITKCPPMICROSERVICES_EXPORT ModuleResource
{

private:

  typedef ModuleResourcePrivate* ModuleResource::*bool_type;

public:

  /**
   * \brief Creates an invalid %ModuleResource object.
   */
  ModuleResource();
  /**
   * \brief Copy constructor.
   * \param[in] resource The object to be copied.
   */
  ModuleResource(const ModuleResource& resource);

  /** \brief Destructor. */
  ~ModuleResource();

  /**
   * \brief Assignment operator.
   *
   * \param[in] resource The %ModuleResource object which is assigned to this instance.
   * \return A reference to this %ModuleResource instance.
   */
  ModuleResource& operator=(const ModuleResource& resource);

  /**
   * \brief A less then operator using the full resource path as returned by
   * GetResourcePath() to define the ordering.
   *
   * \param[in] resource The object to which this %ModuleResource object is compared to.
   * \return \c true if this %ModuleResource object is less then \c resource,
   * \c false otherwise.
   */
  bool operator<(const ModuleResource& resource) const;

  /**
   * \brief Equality operator for %ModuleResource objects.
   *
   * \param[in] resource The object for testing equality.
   * \return \c true if this %ModuleResource object is equal to \c resource, i.e.
   * they are coming from the same module (shared or static) and have an equal
   * resource path, \c false otherwise.
   */
  bool operator==(const ModuleResource& resource) const;

  /**
   * \brief Inequality operator for %ModuleResource objects.
   *
   * \param[in] resource The object for testing inequality.
   * \return The result of <code>!(*this == resource)</code>.
   */
  bool operator!=(const ModuleResource& resource) const;

  /**
   * \brief Tests this %ModuleResource object for validity.
   *
   * Invalid %ModuleResource objects are created by the default constructor or
   * can be returned by the Module class if the resource path is not found.
   *
   * \return \c true if this %ModuleReource object is valid and can safely be used,
   * \c false otherwise.
   */
  bool IsValid() const;

  /**
   * \brief Boolean conversion operator using IsValid().
   */
  operator bool_type() const;

  /**
   * \brief Returns the name of the resource, excluding the path.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string name = resource.GetName(); // name = "archive.tar.gz"
   * \endcode
   *
   * \return The resource name.
   * \sa GetPath(), GetResourcePath()
   */
  std::string GetName() const;

  /**
   * \brief Returns the resource's path, without the file name.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string path = resource.GetPath(); // path = "/data/"
   * \endcode
   *
   * The path with always begin and end with a forward slash.
   *
   * \return The resource path without the name.
   * \sa GetResourcePath(), GetName() and IsDir()
   */
  std::string GetPath() const;

  /**
   * \brief Returns the resource path including the file name.
   *
   * \return The resource path including the file name.
   * \sa GetPath(), GetName() and IsDir()
   */
  std::string GetResourcePath() const;

  /**
   * \brief Returns the base name of the resource without the path.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string base = resource.GetBaseName(); // base = "archive"
   * \endcode
   *
   * \return The resource base name.
   * \sa GetName(), GetSuffix(), GetCompleteSuffix() and GetCompleteBaseName()
   */
  std::string GetBaseName() const;

  /**
   * \brief Returns the complete base name of the resource without the path.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string base = resource.GetCompleteBaseName(); // base = "archive.tar"
   * \endcode
   *
   * \return The resource's complete base name.
   * \sa GetName(), GetSuffix(), GetCompleteSuffix(), and GetBaseName()
   */
  std::string GetCompleteBaseName() const;

  /**
   * \brief Returns the suffix of the resource.
   *
   * The suffix consists of all characters in the resource name after (but not
   * including) the last '.'.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string suffix = resource.GetSuffix(); // suffix = "gz"
   * \endcode
   *
   * \return The resource name suffix.
   * \sa GetName(), GetCompleteSuffix(), GetBaseName() and GetCompleteBaseName()
   */
  std::string GetSuffix() const;

  /**
   * \brief Returns the complete suffix of the resource.
   *
   * The suffix consists of all characters in the resource name after (but not
   * including) the first '.'.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string suffix = resource.GetCompleteSuffix(); // suffix = "tar.gz"
   * \endcode
   *
   * \return The resource name suffix.
   * \sa GetName(), GetSuffix(), GetBaseName(), and GetCompleteBaseName()
   */
  std::string GetCompleteSuffix() const;

  /**
   * \brief Returns \c true if this %ModuleResource object points to a directory and thus
   * may have child resources.
   *
   * \return \c true if this object points to a directory, \c false otherwise.
   */
  bool IsDir() const;

  /**
   * \brief Returns \c true if this %ModuleResource object points to a file resource.
   *
   * \return \c true if this object points to an embedded file, \c false otherwise.
   */
  bool IsFile() const;

  /**
   * \brief Returns a list of resource names which are children of this object.
   *
   * The returned names are relative to the path of this %ModuleResource object
   * and may contain file as well as directory entries.
   *
   * \return A list of child resource names.
   */
  std::vector<std::string> GetChildren() const;

  /**
   * \brief Returns a list of resource objects which are children of this object.
   *
   * The return ModuleResource objects may contain files as well as
   * directory resources.
   *
   * \return A list of child resource objects.
   */
  std::vector<ModuleResource> GetChildResources() const;

  /**
   * \brief Returns the size of the resource data for this %ModuleResource object.
   *
   * \return The resource data size.
   */
  int GetSize() const;

  /**
   * \brief Returns the last modified time of this resource in seconds from the epoch.
   *
   * \return Last modified time of this resource.
   */
  time_t GetLastModified() const;

private:

  ModuleResource(const std::string& file, const ModuleResourceContainer& resourceContainer);
  ModuleResource(int index, const ModuleResourceContainer& resourceContainer);

  friend class Module;
  friend class ModulePrivate;
  friend class ModuleResourceContainer;
  friend class ModuleResourceStream;

  friend struct std::hash<ModuleResource>;

  std::size_t Hash() const;

  void* GetData() const;

  ModuleResourcePrivate* d;

};

}

US_MSVC_POP_WARNING

/**
 * \ingroup MicroServices
 * \brief Stream output operator for ModuleResource.
 *
 * \param[in] os The output stream.
 * \param[in] resource The module resource to write to the stream.
 * \return The output stream.
 */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const us::ModuleResource& resource);

namespace std {
template<> struct hash<us::ModuleResource> { std::size_t operator()(const us::ModuleResource& arg) const {
  return arg.Hash();
} };
}

#endif // USMODULERESOURCE_H
