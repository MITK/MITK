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

#ifndef MITK_COLLECTION_READER_H
#define MITK_COLLECTION_READER_H

#include "mitkCommon.h"
#include "mitkDataCollection.h"
#include <MitkDataCollectionExports.h>

// VTK
#include <vtkXMLParser.h>

namespace mitk {
  class MITKDATACOLLECTION_EXPORT CollectionReader : public vtkXMLParser
  {
  public:

    typedef std::vector<std::vector<std::string> > FileListType;

    /**
    * @brief Build up a mitk::DataCollection from  a XML resource
    *
    **/
    DataCollection::Pointer LoadCollection(const std::string& xmlFileName);

    void AddDataElementIds(std::vector<std::string> dataElemetIds);
    void AddSubColIds(std::vector<std::string> subColIds);

    void SetDataItemNames(std::vector<std::string> itemNames);

    void ClearDataElementIds();
    void ClearSubColIds();

    void Clear();

    /**
    * @brief Build up a mitk::DataCollection from  a folder providing suffixes to group the files
    *
    **/
    static DataCollection::Pointer FolderToCollection(std::string folder, std::vector<std::string> suffixes,std::vector<std::string> seriesNames,  bool allowGaps);

    /**
    * @brief GenerateFileLists Returns a collection of lists with valid files names in a folder
    *
    * The first suffix entry in the vector is used for the reference images which are put in the first list,
    * then the suffixes are interchanged and checked if the file exists; for each suffix a list is returned with filenames,
    * if a file is expected but does not exist an empty string "" is added instead.
    *
    * @param folder
    * @param suffixes
    * @param allowGaps
    * @return
    */
    static FileListType GenerateFileLists(std::string folder, std::vector<std::string> suffixes, bool allowGaps = false);

    /**
    * @brief SanitizeFileList Removes all entries that are lacking at least one modality
    * @param list - sanitized list
    * @return
    */

    static FileListType SanitizeFileList(FileListType list);

    CollectionReader();
    ~CollectionReader();
  protected:

    /**
    * @brief Derived from XMLReader
    **/
    void  StartElement (const char* elementName, const char **atts);
    /**
    * @brief Derived from XMLReader
    **/
    void  EndElement (const char* elementName);

  private:
    /**
    * @brief Derived from XMLReader
    **/
    std::string ReadXMLStringAttribut( std::string name, const char** atts);
    /**
    * @brief Derived from XMLReader
    **/
    bool ReadXMLBooleanAttribut( std::string name, const char** atts );
    /**
    * @brief Derived from XMLReader
    **/
    int ReadXMLIntegerAttribut( std::string name, const char** atts );

    /**
    * @brief m_DataCollection
    * Stores a data collection during build of
    */
    DataCollection::Pointer m_Collection;
    DataCollection::Pointer m_SubCollection;
    DataCollection::Pointer m_DataItemCollection;

    /**
    * @brief m_SelectedIds
    *
    *  Stores ids which are to be considered during loading, if this is provided all
    *  all data sets not matching an ID in this list are skipped.
    */
    std::vector<std::string> m_SelectedSubColIds;

    std::vector<std::string> m_SelectedDataItemIds;

    /**
    * @brief m_SelectedDataItemNames
    * Lists names of items to be loaded, rest is ignored
    */

    std::vector<std::string> m_SelectedDataItemNames;

    /**
    * @brief m_Ignore
    *
    * Determines if current sub-collection is ignored
    */
    bool m_ColIgnore;
    bool m_ItemIgnore;

    /**
    * @brief m_BaseDir
    *
    * Stores Base directory to allow XML links with relative paths
    */
    std::string m_BaseDir;
  };
} // namespace mitk

#endif /* MITK_COLLECTION_READER_H */
