/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneJsonReader_h
#define mitkSceneJsonReader_h

#include <MitkSceneSerializationExports.h>

#include <mitkAbstractSceneReader.h>

namespace mitk
{
  /**
   * \brief Reader for MITK JSON scene files (.mitkscene.json).
   *
   * Loads scenes described in the JSON scene format (root object
   * `{ "type": "org.mitk.scene", "version": 1, "nodes": [...] }`) into a
   * DataStorage. See the format specification page for the full schema.
   *
   * The reader supports two load styles per property map:
   *  - `modify` (default): mapper-assigned defaults are kept; only listed
   *     properties override them.
   *  - `replace`: the target property list is cleared unconditionally
   *    (plain `PropertyList::Clear()`) before the listed properties are
   *    applied. This intentionally differs from the legacy XML reader,
   *    which preserved a small set of mapper-assigned defaults; JSON
   *    authors that need those properties must list them explicitly.
   *
   * The reader is invoked either via SceneIO (for `.mitk` archives
   * containing an `index.json`, or for standalone `.mitkscene.json` files)
   * or directly.
   */
  class MITKSCENESERIALIZATION_EXPORT SceneJsonReader : public AbstractSceneReader
  {
  public:
    mitkClassMacro(SceneJsonReader, AbstractSceneReader);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Load a JSON scene file.
     *
     * \param sceneSourcePath    Absolute path of the `.mitkscene.json` / `index.json`
     *                           file. Its parent directory is the base path for
     *                           resolving relative `_file` references.
     * \param storage            Target DataStorage. Must not be null.
     * \param clearStorageFirst  If true, \p storage is cleared after the scene
     *                           root header (`type`, `version`) has been parsed
     *                           and validated, but before any node is added.
     *                           A malformed or unreadable file leaves the
     *                           existing storage contents untouched.
     *
     * \return True on success; false if non-fatal errors occurred.
     * \throws mitk::Exception on fatal errors.
     */
    bool LoadScene(const std::string &sceneSourcePath, DataStorage *storage, bool clearStorageFirst = false) override;

  protected:
    SceneJsonReader();
    ~SceneJsonReader() override;
  };
}

#endif
