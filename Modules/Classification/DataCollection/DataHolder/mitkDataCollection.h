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
#pragma warning (disable : 4996)

#ifndef mitkDataCollection_H_
#define mitkDataCollection_H_

#include <itkLightObject.h>
#include <itkDataObject.h>
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <mitkImage.h>
#include <mitkDataNode.h>

#include<mitkBaseData.h>

#include <MitkDataCollectionExports.h>

/**
* \brief DataCollection - Class to facilitate loading/accessing structured data
*
* Data is grouped into a collection that may contain further (sub) collections or images.
*
* Exemplary structure
*
* Collection (e.g. Patient)
* |
* |-- Sub-Collection1 (e.g. follow-up 1)
* |   |
* |   |-- DataItem (e.g. T1)
* |   |-- DataItem (e.g. T2)
* |
* |-- Sub-Collection2 (e.g. follow-up 2)
* |   |
* |   |-- DataItem (e.g. T1)
* |   |-- DataItem (e.g. T2)
*
*/

namespace mitk
{
  class MITKDATACOLLECTION_EXPORT DataCollection  : public BaseData
  {
  public:

    mitkClassMacro(DataCollection, BaseData)

      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      // Needed methods from Basedata
      virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion(const itk::DataObject *);

    void Init(std::string name);

    /**
    * @brief AddData Add a data item
    * @param data Images/Fibers/DataCollections
    * @param name name that allows identifying this data (e.g. a category T2, Segmentation , etc ...)
    * @param description
    * @return
    */

    size_t AddData(DataObject::Pointer data, std::string name, std::string filePath = "");

    /**
    * @brief SetName  - Sets name of DataCollection
    * @param name
    */
    void SetName(std::string name);

    std::string GetName() const;

    std::string GetDataFilePath(size_t index) const;

    /**
    * @brief NameToIndex - Get index from data item name
    * @param name
    * @return
    */

    size_t NameToIndex(std::string name);

    /**
    * @brief IndexToName - Get name from index
    * @param index
    * @return
    */
    std::string IndexToName(size_t index) const;

    /**
    * @brief HasElement - check if element with this name exists in collection
    * @param name
    * @return
    */
    bool HasElement(std::string name);

    /**
    * @brief HasElement - check if element with this index exists in collection
    * @param index
    * @return
    */

    bool HasElement(size_t index);

    /**
    * @brief Size - number of data items in collection
    * @return
    */
    size_t Size() const;

    /**
    * @brief SetData - set/update data item by index
    * @param data
    * @param index
    */

    void SetData(itk::DataObject::Pointer data, size_t index);

    /**
    * @brief SetData - set/update data item by name
    * @param data
    * @param name
    */

    void SetData(itk::DataObject::Pointer data, std::string name);

    /**
    * @brief GetData Get original data by index
    *
    *  To ensure a mitk::Image is returned use \ref mitk::GetMitkImage
    *
    * @param index
    * @return
    */
    itk::DataObject::Pointer GetData(size_t index);

    /**
    * @brief GetData Get original data by name
    *
    * To ensure a mitk::Image is returned use \ref mitk::GetMitkImage
    *
    * @param name
    * @return
    */
    itk::DataObject::Pointer GetData(std::string name);

    /**
    * @brief GetMitkImage - casts data to  mitk::Image and returns it
    *
    * \note returns NULL is object  is no mitk::Image or itk::Image
    *
    * @param index
    * @return
    */
    mitk::Image::Pointer GetMitkImage(size_t index);

    /**
    * @brief GetMitkImage - casts data to  mitk::Image and returns it
    *
    * \note returns NULL is object  is no mitk::Image or itk::Image
    *
    * @param name
    * @return
    */
    mitk::Image::Pointer GetMitkImage(std::string name);

    /**
    * @brief GetMitkImage - casts data to privided itk::Image pointer
    */
    template <class ImageType>
    ImageType GetItkImage(size_t index, ImageType* itkImage);

    /**
    * @brief GetMitkImage - casts data to privided itk::Image pointer
    */
    template <class ImageType>
    ImageType GetItkImage(std::string name, ImageType* itkImage);

    itk::DataObject::Pointer& operator[](size_t index);

    itk::DataObject::Pointer& operator[](std::string &name);

    /**
    * @brief SetNameForIndex - sets name for given data item by index
    * @param index
    * @param name
    */
    void SetNameForIndex(size_t index, std::string &name);

    /**
    * @brief SetXMLFile - sets xml file to which data collection is saved
    */
    void SetXMLFile(std::string absoluteXMlFile);

    /**
    * @brief SetXMLFile - gets xml file to which data collection is supposed to be saved
    */
    std::string GetXMLFile();

    /**
    * @brief SetParent - sets the parent collection
    * @param parent
    */
    void SetParent(mitk::DataCollection* parent);

    /**
    * @brief GetParent - returns the parent collection if available else null is returned
    * @return
    */
    mitk::DataCollection* GetParent();

    /**
    * @brief RemoveIndex - removes element at index
    * @param index
    * @return
    */
    bool RemoveIndex(size_t index);

    /**
    * @brief RemoveElement - removes element with name
    * @param name
    * @return
    */
    bool RemoveElement(std::string& name);

    /**
    * @brief Clear - clears the data collection
    */
    void Clear();

    /**
    * @brief GetDataNode - returns data node containing data at index
    * @param index
    * @return
    */
    mitk::DataNode::Pointer GetDataNode(size_t index);

    /**
    * @brief GetDataNode - returns data node containing data with name
    * @param name
    * @return
    */
    mitk::DataNode::Pointer GetDataNode(std::string name);

    /**
    * @brief GetProbabilityMap - returns vectorimage generated out of images with names in the probabilityNamesVector
    * @param probabilityNamesVector
    * @return
    */
    mitk::Image::Pointer GetProbabilityMap(std::vector<std::string> probabilityNamesVector);

  protected:

    DataCollection();
    virtual ~DataCollection();

  private:
    // DATA
    std::string m_Name;
    std::vector<itk::DataObject::Pointer> m_DataVector;
    std::vector<std::string> m_NameVector;
    std::vector<std::string> m_FilePathVector;
    std::map<std::string, size_t> m_DataNames;

    mitk::DataCollection * m_Parent;

    std::string m_XMLFile; // is only filled for the hightest layer when loading a data collection
  };
} // end namespace
#endif
