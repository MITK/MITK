/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAbstractSceneReader_h
#define mitkAbstractSceneReader_h

#include <MitkSceneSerializationExports.h>

#include <itkObject.h>
#include <mitkCommon.h>
#include <mitkDataStorage.h>

#include <string>

namespace mitk
{
  /**
   * \brief Base class for scene-graph aware readers.
   *
   * Scene readers populate a DataStorage with nodes, their data, their
   * parent/child relationships and their (possibly context-specific) property
   * lists. This is structurally richer than what AbstractFileReader's contract
   * can express, which is why scene readers form a separate class family and
   * are invoked through SceneIO rather than the IOUtil / microservice path.
   *
   * Concrete subclasses: SceneJsonReader (JSON scene files). The legacy XML
   * reader (SceneReaderV1) remains on its own interface for now.
   */
  class MITKSCENESERIALIZATION_EXPORT AbstractSceneReader : public itk::Object
  {
  public:
    mitkClassMacroItkParent(AbstractSceneReader, itk::Object);

    /**
     * \brief Load a scene into \p storage.
     *
     * \param sceneSourcePath    Path of the primary scene descriptor (the
     *                           JSON/XML index file). Its parent directory is
     *                           used as the base path for resolving relative
     *                           file references.
     * \param storage            Target DataStorage. Must not be null.
     * \param clearStorageFirst  If true, the reader is responsible for
     *                           clearing \p storage before adding the loaded
     *                           nodes. Clearing must be deferred until after
     *                           the scene descriptor has been parsed and
     *                           validated enough to commit to the load, so
     *                           that unreadable or malformed scene files do
     *                           not wipe the caller's session.
     *
     * \return True on success; false if non-fatal errors occurred during
     *         loading.
     * \throws mitk::Exception on fatal errors (invalid format, unsupported
     *         version, structural inconsistencies, ...).
     *
     * \pre storage != nullptr
     */
    virtual bool LoadScene(const std::string &sceneSourcePath, DataStorage *storage, bool clearStorageFirst = false) = 0;

    /**
     * \brief Collect the nodes this reader adds in the given list.
     *
     * For a caller that has to tell the scene's nodes apart from anything else
     * that reached the same storage. Comparing the storage before and against
     * after does not do that: a load runs on a worker thread while the thread
     * that owns the storage keeps handling events, so a node another handler
     * adds in the meantime is indistinguishable from one of ours.
     *
     * \param loadedNodes The list to append to, or nullptr to collect none.
     *        Not cleared; the caller owns it and it must outlive the load.
     */
    void SetLoadedNodes(DataStorage::SetOfObjects *loadedNodes);

  protected:
    AbstractSceneReader();
    ~AbstractSceneReader() override;

    /** \brief Null unless a caller wants to know which nodes came from here. */
    DataStorage::SetOfObjects *m_LoadedNodes = nullptr;
  };
}

#endif
