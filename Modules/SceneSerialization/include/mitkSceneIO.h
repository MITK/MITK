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

#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"

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

  class MITKSCENESERIALIZATION_EXPORT SceneIO : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SceneIO, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef DataStorage::SetOfObjects FailedBaseDataListType;

    /**
     * \brief Load a scene of objects from file
     * \return DataStorage with all scene objects and their relations. If loading failed, query GetFailedNodes() and
     * GetFailedProperties() for more detail.
     *
     * Attempts to read the provided file and create objects with
     * parent/child relations into a DataStorage.
     *
     * \param filename full filename of the scene file
     * \param storage If given, this DataStorage is used instead of a newly created one
     * \param clearStorageFirst If set, the provided DataStorage will be cleared before populating it with the loaded
     * objects
     */
    virtual DataStorage::Pointer LoadScene(const std::string &filename,
                                           DataStorage *storage = nullptr,
                                           bool clearStorageFirst = false);

    /**
    * \brief Load a scene of objects from directory.
    * \return DataStorage with all scene objects and their relations. If loading failed, query GetFailedNodes() and
    * GetFailedProperties() for more detail.
    *
    * Does the same like LoadScene, but assumes that the given filename is the index.xml of the scene and the working directory
    * is the directory of the given filename. This function can be used to load an already unpacked scene and create objects with
    * parent/child relations into a DataStorage.
    *
    * \param indexfilename full filename of the scene index file
    * \param storage If given, this DataStorage is used instead of a newly created one
    * \param clearStorageFirst If set, the provided DataStorage will be cleared before populating it with the loaded
    * objects
    */
    virtual DataStorage::Pointer LoadSceneUnzipped(const std::string &indexfilename,
      DataStorage *storage = nullptr,
      bool clearStorageFirst = false);


    /**
     * \brief Save a scene of objects to file
     * \return True if complete success, false if any problem occurred. Note that a scene file might still be written if
     false is returned,
               it just will not contain every node/property. If writing failed, query GetFailedNodes() and
     GetFailedProperties() for more detail.
     *
     * Attempts to write a scene file, which contains the nodes of the
     * provided DataStorage, their parent/child relations, and properties.
     *
     * \param sceneNodes
     * \param storage a DataStorage containing all nodes that should be saved
     * \param filename
     */
    virtual bool SaveScene(DataStorage::SetOfObjects::ConstPointer sceneNodes,
                           const DataStorage *storage,
                           const std::string &filename);

    /**
     * \brief Get a list of nodes (BaseData containers) that failed to be read/written.
     *
     * FailedBaseDataListType hold all those nodes that contain BaseData objects
     * which could not be read or written during the last call to LoadScene or SaveScene.
     */
    const FailedBaseDataListType *GetFailedNodes();

    /**
     * \brief Get a list of properties that failed to be read/written.
     *
     * Each entry corresponds to a property which could not
     * be (de)serialized. The properties may come from either of
     * <ul>
     *   <li> The BaseData's PropertyList
     *   <li> The DataNodes's PropertyList
     *   <li> Any of a DataNodes's render window specific PropertyLists
     * </ul>
     */
    const PropertyList *GetFailedProperties();

  protected:
    SceneIO();
    ~SceneIO() override;

    std::string CreateEmptyTempDirectory();

    tinyxml2::XMLElement *SaveBaseData(tinyxml2::XMLDocument &doc, BaseData *data, const std::string &filenamehint, bool &error);
    tinyxml2::XMLElement *SavePropertyList(tinyxml2::XMLDocument &doc, PropertyList *propertyList, const std::string &filenamehint);

    void OnUnzipError(const void *pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> &info);
    void OnUnzipOk(const void *pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path> &info);

    FailedBaseDataListType::Pointer m_FailedNodes;
    PropertyList::Pointer m_FailedProperties;

    std::string m_WorkingDirectory;
    unsigned int m_UnzipErrors;
  };
}

#endif
