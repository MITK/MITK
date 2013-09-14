/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/


#ifndef USMODULERESOURCE_H
#define USMODULERESOURCE_H

#include <usConfig.h>

#include <ostream>
#include <vector>

US_MSVC_PUSH_DISABLE_WARNING(4396)

US_BEGIN_NAMESPACE

class ModuleResourcePrivate;
class ModuleResourceTree;

/**
 * \ingroup MicroServices
 *
 * Represents a resource (text file, image, etc.) embedded in a CppMicroServices module.
 *
 * A \c %ModuleResource object provides information about a resource (external file) which
 * was embedded into this module's shared library. \c %ModuleResource objects can be obtained
 * be calling Module#GetResource or Module#FindResources.
 *
 * Example code for retreiving a resource object and reading its contents:
 * \snippet uServices-resources/main.cpp 1
 *
 * %ModuleResource objects have value semantics and copies are very inexpensive.
 *
 * \see ModuleResourceStream
 * \see \ref MicroServices_Resources
 */
class US_EXPORT ModuleResource
{

private:

  typedef ModuleResourcePrivate* ModuleResource::*bool_type;

public:

  /**
   * Creates in invalid %ModuleResource object.
   */
  ModuleResource();
  /**
   * Copy constructor.
   * @param resource The object to be copied.
   */
  ModuleResource(const ModuleResource& resource);

  ~ModuleResource();

  /**
   * Assignment operator.
   *
   * @param resource The %ModuleResource object which is assigned to this instance.
   * @return A reference to this %ModuleResource instance.
   */
  ModuleResource& operator=(const ModuleResource& resource);

  /**
   * A less then operator using the full resource path as returned by
   * GetResourcePath() to define the ordering.
   *
   * @param resource The object to which this %ModuleResource object is compared to.
   * @return \c true if this %ModuleResource object is less then \c resource,
   * \c false otherwise.
   */
  bool operator<(const ModuleResource& resource) const;

  /**
   * Equality operator for %ModuleResource objects.
   *
   * @param resource The object for testing equality.
   * @return \c true if this %ModuleResource object is equal to \c resource, i.e.
   * they are coming from the same module (shared or static) and have an equal
   * resource path, \c false otherwise.
   */
  bool operator==(const ModuleResource& resource) const;

  /**
   * Inequality operator for %ModuleResource objects.
   *
   * @param resource The object for testing inequality.
   * @return The result of <code>!(*this == resource)</code>.
   */
  bool operator!=(const ModuleResource& resource) const;

  /**
   * Tests this %ModuleResource object for validity.
   *
   * Invalid %ModuleResource objects are created by the default constructor or
   * can be returned by the Module class if the resource path is not found. If a
   * module from which %ModuleResource objects have been obtained is un-loaded,
   * these objects become invalid.
   *
   * @return \c true if this %ModuleReource object is valid and can safely be used,
   * \c false otherwise.
   */
  bool IsValid() const;

  /**
   * Returns \c true if the resource represents a file and the resource data
   * is in a compressed format, \c false otherwise.
   *
   * @return \c true if the resource data is compressed, \c false otherwise.
   */
  bool IsCompressed() const;

  /**
   * Boolean conversion operator using IsValid().
   */
  operator bool_type() const;

  /**
   * Returns the name of the resource, excluding the path.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string name = resource.GetName(); // name = "archive.tar.gz"
   * \endcode
   *
   * @return The resource name.
   * @see GetPath(), GetResourcePath()
   */
  std::string GetName() const;

  /**
   * Returns the resource's path, without the file name.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string path = resource.GetPath(); // path = "/data"
   * \endcode
   *
   * @return The resource path without the name.
   * @see GetResourcePath(), GetName() and IsDir()
   */
  std::string GetPath() const;

  /**
   * Returns the resource path including the file name.
   *
   * @return The resource path including the file name.
   * @see GetPath(), GetName() and IsDir()
   */
  std::string GetResourcePath() const;

  /**
   * Returns the base name of the resource without the path.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string base = resource.GetBaseName(); // base = "archive"
   * \endcode
   *
   * @return The resource base name.
   * @see GetName(), GetSuffix(), GetCompleteSuffix() and GetCompleteBaseName()
   */
  std::string GetBaseName() const;

  /**
   * Returns the complete base name of the resource without the path.
   *
   * Example:
   * \code
   * ModuleResource resource = module->GetResource("/data/archive.tar.gz");
   * std::string base = resource.GetCompleteBaseName(); // base = "archive.tar"
   * \endcode
   *
   * @return The resource's complete base name.
   * @see GetName(), GetSuffix(), GetCompleteSuffix(), and GetBaseName()
   */
  std::string GetCompleteBaseName() const;

  /**
   * Returns the suffix of the resource.
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
   * @return The resource name suffix.
   * @see GetName(), GetCompleteSuffix(), GetBaseName() and GetCompleteBaseName()
   */
  std::string GetSuffix() const;

  /**
   * Returns the complete suffix of the resource.
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
   * @return The resource name suffix.
   * @see GetName(), GetSuffix(), GetBaseName(), and GetCompleteBaseName()
   */
  std::string GetCompleteSuffix() const;

  /**
   * Returns \c true if this %ModuleResource object points to a directory and thus
   * may have child resources.
   *
   * @return \c true if this object points to a directory, \c false otherwise.
   */
  bool IsDir() const;

  /**
   * Returns \c true if this %ModuleResource object points to a file resource.
   *
   * @return \c true if this object points to an embedded file, \c false otherwise.
   */
  bool IsFile() const;

  /**
   * Returns a list of resource names which are children of this object.
   *
   * The returned names are relative to the path of this %ModuleResource object and
   * may contain duplicates in case of modules which are statically linked into the
   * module from which this object was retreived.
   *
   * @return A list of child resource names.
   */
  std::vector<std::string> GetChildren() const;

  /**
   * Returns the size of the raw embedded data for this %ModuleResource object.
   *
   * @return The resource data size.
   */
  int GetSize() const;

  /**
   * Returns a data pointer pointing to the raw data of this %ModuleResource object.
   * If the resource is compressed the data returned is compressed and UncompressResourceData()
   * must be used to access the data. If the resource represents a directory \c 0 is returned.
   *
   * @return A raw pointer to the embedded data, or \c 0 if the resource is not a file resource.
   */
  const unsigned char* GetData() const;

private:

  ModuleResource(const std::string& file, ModuleResourceTree* resourceTree,
                 const std::vector<ModuleResourceTree*>& resourceTrees);

  friend class Module;
  friend class ModulePrivate;

  US_HASH_FUNCTION_FRIEND(ModuleResource);

  std::size_t Hash() const;

  ModuleResourcePrivate* d;

};

US_END_NAMESPACE

US_MSVC_POP_WARNING

/**
 * \ingroup MicroServices
 */
US_EXPORT std::ostream& operator<<(std::ostream& os, const US_PREPEND_NAMESPACE(ModuleResource)& resource);

US_HASH_FUNCTION_NAMESPACE_BEGIN
US_HASH_FUNCTION_BEGIN(US_PREPEND_NAMESPACE(ModuleResource))
  return arg.Hash();
US_HASH_FUNCTION_END
US_HASH_FUNCTION_NAMESPACE_END

#endif // USMODULERESOURCE_H
