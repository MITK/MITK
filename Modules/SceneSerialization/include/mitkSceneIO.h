/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneIO_h
#define mitkSceneIO_h

#include <MitkSceneSerializationExports.h>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <Poco/Zip/ZipLocalFileHeader.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{
  class BaseData;
  class PropertyList;

  /**
   * \brief Provides functionality to load and save MITK scene files (.mitk).
   *
   * SceneIO handles the complete scene serialization pipeline:
   * - Loading: Unzips a .mitk scene file, parses index.xml, and reconstructs
   *   DataNodes with their data, properties, and parent/child relationships
   *   into a DataStorage.
   * - Saving: Serializes DataNodes from a DataStorage into temporary files,
   *   writes an index.xml, and packages everything into a ZIP archive.
   *
   * Scene files (.mitk) are ZIP archives containing:
   * - An index.xml file describing all nodes, their relationships, and references
   *   to serialized data and property files.
   * - Serialized BaseData files (images, surfaces, etc.).
   * - Serialized PropertyList XML files.
   *
   * After loading or saving, failed nodes and properties can be queried to
   * determine what could not be processed.
   *
   * \sa SceneReader, BaseDataSerializer, PropertyListSerializer
   */
  class MITKSCENESERIALIZATION_EXPORT SceneIO : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SceneIO, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Type for a list of DataNodes whose BaseData failed to serialize/deserialize. */
      typedef DataStorage::SetOfObjects FailedBaseDataListType;

    /**
     * \brief Loads a scene from an MITK scene file.
     *
     * Accepts either a `.mitk` ZIP archive (unpacked to a temporary
     * directory, then dispatched via index.json if present, else
     * index.xml) or a standalone `.mitkscene.json` file.
     *
     * \param[in] filename         Full path to the scene file.
     * \param[in] storage          If non-null, this DataStorage is populated instead of
     *                             creating a new StandaloneDataStorage.
     * \param[in] clearStorageFirst If true, the provided DataStorage is cleared before
     *                             loading new objects into it. For the JSON path,
     *                             clearing is deferred until after the scene
     *                             descriptor has been validated so that a malformed
     *                             file does not wipe the caller's session.
     *
     * \return A DataStorage containing all successfully loaded scene objects and
     *         their relationships. Per-node load failures are reported via
     *         MITK_ERROR log output; GetFailedNodes() / GetFailedProperties()
     *         reflect save-side failures only and are not populated here.
     *
     * \note For the JSON path, this method does not throw: exceptions
     *       raised by SceneJsonReader (JSON parse errors, missing data
     *       files, property-map resolution errors) are caught and logged
     *       as MITK_ERROR, and the returned DataStorage may be empty or
     *       partially populated. The legacy XML path retains its existing
     *       behavior and may propagate exceptions from the underlying
     *       reader; callers that need to handle both formats uniformly
     *       should wrap the call in their own try/catch.
     *
     * \post The temporary directory is deleted after loading.
     */
    virtual DataStorage::Pointer LoadScene(const std::string &filename,
                                           DataStorage *storage = nullptr,
                                           bool clearStorageFirst = false);

    /**
     * \brief Loads a scene from an already-unpacked directory.
     *
     * Similar to LoadScene(), but operates on an unpacked scene directory
     * rather than a ZIP archive. Assumes the given file is the index.xml
     * of the scene and uses its parent directory as the working directory.
     *
     * \param[in] indexfilename    Full path to the scene's index.xml file.
     * \param[in] storage          If non-null, this DataStorage is populated instead of
     *                             creating a new StandaloneDataStorage.
     * \param[in] clearStorageFirst If true, the provided DataStorage is cleared before
     *                             loading new objects into it.
     *
     * \return A DataStorage containing all successfully loaded scene objects and
     *         their relationships. Per-node load failures are reported via
     *         MITK_ERROR log output; GetFailedNodes() / GetFailedProperties()
     *         reflect save-side failures only and are not populated here.
     */
    virtual DataStorage::Pointer LoadSceneUnzipped(const std::string &indexfilename,
      DataStorage *storage = nullptr,
      bool clearStorageFirst = false);


    /**
     * \brief Saves a scene of DataNodes to a .mitk scene file (ZIP archive).
     *
     * Serializes the given set of DataNodes (including their data, properties,
     * and parent/child relationships from the DataStorage) into a temporary
     * directory, creates an index.xml, and packages everything into a ZIP archive
     * at the specified filename.
     *
     * \param[in] sceneNodes The set of DataNodes to save.
     * \param[in] storage    The DataStorage containing the nodes and their relationships.
     * \param[in] filename   Full path for the output .mitk scene file.
     *
     * \return True if the scene was saved completely and successfully. False if
     *         any problem occurred. Note that a partial scene file may still be
     *         written. Query GetFailedNodes() and GetFailedProperties() for details.
     *
     * \pre \p sceneNodes must not be null.
     * \pre \p storage must not be null.
     * \pre \p filename must not be empty.
     */
    virtual bool SaveScene(DataStorage::SetOfObjects::ConstPointer sceneNodes,
                           const DataStorage *storage,
                           const std::string &filename);

    /**
     * \brief Returns DataNodes whose BaseData failed to be written during the
     *        most recent SaveScene() call.
     *
     * \note These accessors currently reflect save-side failures only. Load
     *       paths (both the legacy XML reader and SceneJsonReader) report
     *       per-node errors via MITK_ERROR log output and the reader's return
     *       value, not via this list.
     *
     * \return Pointer to the list of failed nodes, or nullptr if none failed.
     */
    const FailedBaseDataListType *GetFailedNodes();

    /**
     * \brief Returns properties that failed to be written during the most
     *        recent SaveScene() call.
     *
     * The properties may originate from:
     * - The BaseData's PropertyList
     * - The DataNode's PropertyList
     * - Any of a DataNode's render-window-specific PropertyLists
     *
     * \note See GetFailedNodes() — load paths do not populate this list.
     *
     * \return Pointer to the PropertyList of failed properties, or nullptr if none failed.
     */
    const PropertyList *GetFailedProperties();

  protected:
    SceneIO();
    ~SceneIO() override;

    std::string CreateEmptyTempDirectory();

    tinyxml2::XMLElement *SaveBaseData(tinyxml2::XMLDocument &doc, BaseData *data, const std::string &filenamehint, bool &error);
    tinyxml2::XMLElement *SavePropertyList(tinyxml2::XMLDocument &doc, PropertyList *propertyList, const BaseData *nodeData, const std::string &filenamehint);

    void OnUnzipError(const void *pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> &info);
    void OnUnzipOk(const void *pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> &info);

    FailedBaseDataListType::Pointer m_FailedNodes;
    PropertyList::Pointer m_FailedProperties;

    std::string m_WorkingDirectory;
    unsigned int m_UnzipErrors;
  };
}

#endif
