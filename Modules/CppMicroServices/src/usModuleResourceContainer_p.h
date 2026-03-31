/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULERESOURCECONTAINER_P_H
#define USMODULERESOURCECONTAINER_P_H

#include <usGlobalConfig.h>
#include <cstdint>

#include <ctime>
#include <string>
#include <vector>

namespace us {

struct ModuleInfo;
class ModuleResource;
struct ModuleResourceContainerPrivate;

/**
 * \brief Internal container providing access to resources embedded in a module.
 *
 * This class is not part of the public API. It wraps a zip archive that
 * is appended to the module's shared library and provides lookup, stat,
 * and decompression operations for embedded resources.
 *
 * \sa ModuleResource ModulePrivate
 */
class ModuleResourceContainer
{

public:

  /**
   * \brief Construct a resource container for the given module.
   *
   * \param[in] moduleInfo Module metadata used to locate the embedded resource archive.
   */
  ModuleResourceContainer(const ModuleInfo* moduleInfo);

  /** \brief Destructor. */
  ~ModuleResourceContainer();

  /**
   * \brief File-level metadata for a single embedded resource entry.
   */
  struct Stat
  {
    /** \brief Construct with default (invalid) values. */
    Stat()
      : index(-1)
      , uncompressedSize(0)
      , modifiedTime(0)
      , isDir(false)
    {}

    std::string filePath;  ///< \brief Path of the resource inside the archive.
    int index;             ///< \brief Index within the archive (-1 if invalid).
    int uncompressedSize;  ///< \brief Uncompressed size in bytes.
    time_t modifiedTime;   ///< \brief Last modification time.
    bool isDir;            ///< \brief Whether this entry is a directory.
  };

  /**
   * \brief Check whether the resource container was loaded successfully.
   *
   * \return \c true if the container is valid and usable.
   */
  bool IsValid() const;

  /**
   * \brief Fill \a stat with metadata for the resource identified by stat.filePath.
   *
   * \param[in,out] stat On input, filePath must be set; on output, all fields are populated.
   * \return \c true if the entry was found.
   */
  bool GetStat(Stat& stat) const;

  /**
   * \brief Fill \a stat with metadata for the resource at the given archive index.
   *
   * \param[in] index Archive index of the entry.
   * \param[out] stat Populated metadata.
   * \return \c true if the index is valid.
   */
  bool GetStat(int index, Stat& stat) const;

  /**
   * \brief Return a pointer to the decompressed data for the given archive index.
   *
   * \param[in] index Archive index of the entry.
   * \return Pointer to the data, or \c nullptr on failure.
   */
  void* GetData(int index) const;

  /**
   * \brief Return the ModuleInfo this container was created from.
   *
   * \return Pointer to the module info.
   */
  const ModuleInfo* GetModuleInfo() const;

  /**
   * \brief Collect child entries under the given resource path.
   *
   * \param[in] resourcePath Parent directory path to query.
   * \param[in] relativePaths If \c true, return relative names; otherwise absolute.
   * \param[out] names Child entry names.
   * \param[out] indices Corresponding archive indices.
   */
  void GetChildren(const std::string& resourcePath, bool relativePaths,
                   std::vector<std::string>& names, std::vector<uint32_t>& indices) const;

  /**
   * \brief Recursively or non-recursively find resources matching a pattern.
   *
   * \param[in] path Directory path to search.
   * \param[in] filePattern Glob-style pattern for matching file names.
   * \param[in] recurse Whether to search subdirectories.
   * \param[out] resources Matching resources are appended here.
   */
  void FindNodes(const std::string& path, const std::string& filePattern,
                 bool recurse, std::vector<ModuleResource>& resources) const;

private:

  /**
   * \brief Check if a resource name matches the given glob pattern.
   *
   * \param[in] name Resource name to test.
   * \param[in] filePattern Glob pattern.
   * \return \c true if the name matches.
   */
  bool Matches(const std::string& name, const std::string& filePattern) const;

  ModuleResourceContainerPrivate* d; ///< \brief Private implementation data.

};


}

#endif // USMODULERESOURCECONTAINER_P_H
