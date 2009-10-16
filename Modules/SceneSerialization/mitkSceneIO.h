/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkSceneIO_h_included
#define mitkSceneIO_h_included

#include "SceneSerializationExports.h"

#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"

#include <Poco/Zip/ZipLocalFileHeader.h>

class TiXmlElement;

namespace mitk
{

class BaseData;
class PropertyList;

class SceneSerialization_EXPORT SceneIO : public itk::Object
{
  public:

    mitkClassMacro( SceneIO, itk::Object );
    itkNewMacro(Self);

    typedef DataStorage::SetOfObjects                                FailedBaseDataListType;

    /**
     * \brief Load a scene of objects from file
     * \return DataStorage with all scene objects and their relations. If loading failed, query GetFailedNodes() and GetFailedProperties() for more detail.
     *
     * Attempts to read the provided file and create objects with 
     * parent/child relations into a DataStorage.
     * 
     * \param filename full filename of the scene file
     * \param storage If given, this DataStorage is used instead of a newly created one
     * \param clearStorageFirst If set, the provided DataStorage will be cleared before populating it with the loaded objects
     */
    virtual DataStorage::Pointer LoadScene( const std::string& filename, 
                                            DataStorage* storage = NULL, 
                                            bool clearStorageFirst = false );

    /**
     * \brief Save a scene of objects to file
     * \return True if complete success, false if any problem occurred. Note that a scene file might still be written if false is returned,
               it just will not contain every node/property. If writing failed, query GetFailedNodes() and GetFailedProperties() for more detail.
     *
     * Attempts to write a scene file, which contains the nodes of the
     * provided DataStorage, their parent/child relations, and properties.
     *
     * \param storage a DataStorage containing all nodes that should be saved
     * \param filename full filename of the scene file
     * \param predicate defining which items of the datastorage to use and which not
     */
    virtual bool SaveScene( DataStorage::SetOfObjects::ConstPointer sceneNodes, const DataStorage* storage,
                            const std::string& filename);

    /** 
     * \brief Get a list of nodes (BaseData containers) that failed to be read/written.
     *
     * FailedBaseDataListType hold all those nodes that contain BaseData objects
     * which could not be read or written during the last call to LoadScene or SaveScene.
     */
    const FailedBaseDataListType*   GetFailedNodes();

    /** 
     * \brief Get a list of properties that failed to be read/written.
     *
     * Each entry corresponds to a property which could not
     * be (de)serialized. The properties may come from either of
     * <ul>
     *   <li> The BaseData's PropertyList
     *   <li> The DataTreeNodes's PropertyList
     *   <li> Any of a DataTreeNodes's render window specific PropertyLists
     * </ul>
     */
    const PropertyList* GetFailedProperties();

  protected:
    
    SceneIO();
    virtual ~SceneIO();

    std::string CreateEmptyTempDirectory();

    TiXmlElement* SaveBaseData( BaseData* data, const std::string& filenamehint, bool& error);
    TiXmlElement* SavePropertyList( PropertyList* propertyList, const std::string& filenamehint );

    void OnUnzipError(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& info);
    void OnUnzipOk(const void* pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& info);

    FailedBaseDataListType::Pointer m_FailedNodes;
    PropertyList::Pointer           m_FailedProperties;

    std::string  m_WorkingDirectory;
    unsigned int m_UnzipErrors;
};

}

#endif

