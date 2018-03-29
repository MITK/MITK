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
#ifdef _MSC_VER
#  pragma warning (disable : 4996)
#endif

#include "mitkCollectionReader.h"
#include <vtkXMLDataElement.h>

#include <mitkIOUtil.h>
//#include <mitkBaseDataIOFactory.h>

#include <QDir>

//XML StateMachine Tags
// Objects
const std::string COLLECTION = "col";
const std::string SUBCOLLECTION = "subcol";
const std::string DATA = "data";
const std::string ITEM = "item";
// Properties
const std::string NAME = "name";
const std::string ID = "id";
const std::string FILEPATH = "description";
const std::string LINK = "link";


static std::string GetName(std::string fileName,std::string suffix)
{
  fileName = QFileInfo(QString::fromStdString(fileName)).fileName().toStdString();
  return fileName.substr(0,fileName.length() -suffix.length()-9); // 8 = date length
}

static std::string GetDate(std::string fileName,std::string suffix)
{
  fileName = QFileInfo(QString::fromStdString(fileName)).fileName().toStdString();
  fileName = fileName.substr(fileName.length() - suffix.length()-8,8); // 8 = date length
  fileName.insert(6,"-");
  fileName.insert(4,"-");
  return fileName;
}


mitk::CollectionReader::CollectionReader()
 : m_Collection(nullptr),
   m_SubCollection(nullptr),
   m_DataItemCollection(nullptr),
   m_ColIgnore(false), m_ItemIgnore(false)
{
}

mitk::CollectionReader::~CollectionReader()
{
  this->Clear();
}


/**
 * @brief Loads the xml file filename and generates the necessary instances.
 **/
mitk::DataCollection::Pointer mitk::CollectionReader::LoadCollection(const std::string& xmlFileName)
{
  QDir fileName = QFileInfo(xmlFileName.c_str()).absoluteDir();
  m_BaseDir = fileName.path().toStdString() + QDir::separator().toLatin1();
  this->SetFileName(xmlFileName.c_str());
  this->Parse();
  if (m_Collection.IsNotNull())
    m_Collection->SetXMLFile(xmlFileName);
  return m_Collection;
}

void mitk::CollectionReader::AddDataElementIds(std::vector<std::string> dataElemetIds)
{
  m_SelectedDataItemIds.insert( m_SelectedDataItemIds.end(), dataElemetIds.begin(), dataElemetIds.end() );
}

void mitk::CollectionReader::AddSubColIds(std::vector<std::string> subColIds)
{
  m_SelectedSubColIds.insert( m_SelectedSubColIds.end(), subColIds.begin(), subColIds.end() );
}

void mitk::CollectionReader::SetDataItemNames(std::vector<std::string> itemNames)
{
  m_SelectedDataItemNames = itemNames;
}

void mitk::CollectionReader::ClearDataElementIds()
{
  m_SelectedDataItemIds.clear();
}

void mitk::CollectionReader::ClearSubColIds()
{
  m_SelectedSubColIds.clear();
}

void mitk::CollectionReader::Clear()
{
  m_DataItemCollection = nullptr;
  m_SubCollection = nullptr;
  m_Collection = nullptr;
}

mitk::DataCollection::Pointer mitk::CollectionReader::FolderToCollection(std::string folder, std::vector<std::string> suffixes,std::vector<std::string> seriesNames,  bool allowGaps)
{
  // Parse folder and look up all data,
  // after sanitation only fully available groups are included (that is all suffixes are found)
  FileListType fileList = SanitizeFileList(GenerateFileLists(folder, suffixes, allowGaps));

  if (fileList.size() <= 0)
    return nullptr;

  DataCollection::Pointer collection = DataCollection::New();
  collection->SetName(GetName(fileList.at(0).at(0),suffixes.at(0)));

  for (unsigned int k=0; k < fileList.at(0).size(); ++k) // all groups have the same amount of items, so looking at 0 is ok.
  {
    DataCollection::Pointer subCollection = DataCollection::New();
    for (unsigned int i=0; i< suffixes.size(); ++i)
    {
      auto image = IOUtil::Load<Image>(fileList.at(i).at(k));
      subCollection->AddData(image.GetPointer(),seriesNames.at(i), fileList.at(i).at(k));
    }
    std::string sDate =  GetDate(fileList.at(0).at(k),suffixes.at(0));
    collection->AddData(subCollection.GetPointer(),sDate,"--");
  }
  return collection;
}

void mitk::CollectionReader::StartElement(const char* elementName, const char **atts)
{
  std::string name(elementName);

  if (name == COLLECTION)
  {
    m_Collection = DataCollection::New();
    std::string colName = ReadXMLStringAttribut(NAME, atts);
    m_Collection->SetName(colName);
  }
  else if (name == SUBCOLLECTION)
  {
    m_ColIgnore = false;
    m_ItemIgnore = false;

    std::string subColName = ReadXMLStringAttribut(NAME, atts);
    std::string subColId = ReadXMLStringAttribut(ID, atts);

    if (m_SelectedSubColIds.size() > 0  && std::find(m_SelectedSubColIds.begin(), m_SelectedSubColIds.end(), subColId) == m_SelectedSubColIds.end() )
    { // a) a selection list is provided AND b) the item is not in the list
      m_ColIgnore = true;
      return;
    }

    // Create subcollection
    m_SubCollection = DataCollection::New();
    m_SubCollection->Init(subColName);
  }
  else if (name == DATA)
  {
    if (m_ColIgnore)
      return;

    std::string dataId = ReadXMLStringAttribut(ID, atts);
    if (m_SelectedDataItemIds.size() > 0  && std::find(m_SelectedDataItemIds.begin(), m_SelectedDataItemIds.end(), dataId) == m_SelectedDataItemIds.end() )
    { // a) a selection list is provided AND b) the item is not in the list
      m_ItemIgnore = true;
      return;
    }
    m_ItemIgnore = false;
    std::string dataName = ReadXMLStringAttribut(NAME, atts);

    m_DataItemCollection = DataCollection::New();
    m_DataItemCollection->Init(dataName);
  }
  else if (name == ITEM)
  {
    if (m_ColIgnore || m_ItemIgnore)
      return;


    std::string relativeItemLink =  ReadXMLStringAttribut(LINK, atts);
    std::string itemLink = m_BaseDir + relativeItemLink;
    std::string itemName = ReadXMLStringAttribut(NAME, atts);

    // if item names are provided and name is not in list, do not load it
    if (m_SelectedDataItemNames.size() != 0 && std::find(m_SelectedDataItemNames.begin(), m_SelectedDataItemNames.end(), itemName) == m_SelectedDataItemNames.end() )
      return;

    // Populate Sub-Collection
    auto image = IOUtil::Load<Image>(itemLink);
    if (image.IsNotNull())
      m_DataItemCollection->AddData(image.GetPointer(),itemName,relativeItemLink);
    else
      MITK_ERROR << "File could not be loaded: " << itemLink << ". Wihtin Sub-Collection " << m_SubCollection->GetName() << ", within " <<  m_DataItemCollection->GetName() ;
  }
  else
    MITK_WARN<< "Malformed description ? --  unknown tag: " << name;
}

void mitk::CollectionReader::EndElement(const char* elementName)
{
  std::string name(elementName);
  if (name == SUBCOLLECTION)
  {
    if (m_SubCollection.IsNull())
      return;
    if (m_ColIgnore || m_SubCollection->Size() == 0)
      return;

    m_Collection->AddData(m_SubCollection.GetPointer(),m_SubCollection->GetName());
    m_SubCollection = DataCollection::New();
  }
  if (name == DATA)
  {
    if (m_DataItemCollection.IsNull())
      return;
    if (m_DataItemCollection->Size() == 0)
      return;

    m_SubCollection->AddData(m_DataItemCollection.GetPointer(),m_DataItemCollection->GetName());
    m_DataItemCollection = DataCollection::New();
  }
}

std::string mitk::CollectionReader::ReadXMLStringAttribut(std::string name, const char** atts)
{
  if (atts)
  {
    const char** attsIter = atts;

    while (*attsIter)
    {
      if (name == *attsIter)
      {
        attsIter++;
        return *attsIter;
      }
      attsIter++;
      attsIter++;
    }
  }
  return std::string();
}

bool mitk::CollectionReader::ReadXMLBooleanAttribut(std::string name, const char** atts)
{
  std::string s = ReadXMLStringAttribut(name, atts);
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  if (s == "TRUE")
    return true;
  else
    return false;
}


int mitk::CollectionReader::ReadXMLIntegerAttribut(std::string name, const char** atts)
{
  std::string s = ReadXMLStringAttribut(name, atts);
  return atoi(s.c_str());
}



mitk::CollectionReader::FileListType mitk::CollectionReader::GenerateFileLists(std::string folder, std::vector<std::string> suffixes, bool allowGaps)
{
  FileListType fileList;
  QString qFolder = QString::fromStdString(folder);
  if (!QFileInfo(qFolder).isDir())
  {
    MITK_ERROR << "Folder does not exist.";
    return fileList;
  }
  // Add vector for each suffix
  for (unsigned int i=0; i< suffixes.size(); ++i)
  {
    std::vector<std::string> list;
    fileList.push_back(list);
  }

  // if gaps are allowed, file names are build up from reference file (first suffix)
  // else all lists a file, file by file with regular sorting of the files,
  // if one suffix has more/less images than the others loading is aborted
  if (allowGaps)
  {
    QDir parseDir;
    parseDir.setFilter(QDir::Files);
    parseDir.setPath(qFolder);
    QStringList filterMorph;
    filterMorph << "*" + QString::fromStdString( suffixes.at(0) );
    parseDir.setNameFilters( filterMorph );

    QFileInfoList qFileList = parseDir.entryInfoList();

    // now populate lists with files names, non-existing files will be marked with an empty string
    for (int i = 0; i < qFileList.size(); ++i)
    {
      std::string baseFileName = qFileList.at(i).absoluteFilePath().toStdString();
      fileList.at(0).push_back( baseFileName );

      //check for different suffixes
      for (unsigned int suffNo=1; suffNo < suffixes.size(); ++suffNo)
      {
        std::string derivedFileName = baseFileName.substr(0,baseFileName.length() -suffixes.at(0).length()) + suffixes.at(suffNo);

        // checking if file exists
        if (QFileInfo(QString::fromStdString(derivedFileName)).isFile())
          fileList.at(suffNo).push_back(derivedFileName);
        else
          fileList.at(suffNo).push_back("");
      }
    }
  }
  else
  {
    int numberOfFiles=-1;
    for (unsigned int i=0; i< suffixes.size(); ++i)
    {
      QDir parseDir;
      parseDir.setFilter(QDir::Files);
      parseDir.setPath(qFolder);
      QStringList filterMorph;
      filterMorph << "*" + QString::fromStdString( suffixes.at(i) );
      parseDir.setNameFilters( filterMorph );

      QFileInfoList qFileList = parseDir.entryInfoList();
      if (numberOfFiles == -1)
        numberOfFiles = qFileList.size();

      if (numberOfFiles != qFileList.size() )
      {
        MITK_ERROR << "Series contain different number of images. Loading aborting.";
        fileList.clear();
        break;
      }

      for (int fileNo=0; fileNo<qFileList.size(); ++fileNo)
      {
        fileList.at(i).push_back(qFileList.at(fileNo).absoluteFilePath().toStdString());
      }
    }
  }
  return fileList;
}

mitk::CollectionReader::FileListType mitk::CollectionReader::SanitizeFileList(mitk::CollectionReader::FileListType list)
{
  std::vector<int> indexRemoval;
  // Parse through all items and check for empty strings, if one occurs mark this index
  // for removal.
  int modalities = list.size();
  int timeSteps = list.at(0).size();
  MITK_INFO << "Modalities " << modalities;
  MITK_INFO << "TimeSteps " << timeSteps;
  if (timeSteps == 0)
    MITK_ERROR << "No files found. Fatal.";
  for (int listIndex = 0 ; listIndex < timeSteps; listIndex++)
  {
    for (int modalityIndex = 0 ; modalityIndex < modalities; modalityIndex++)
    {
      if (list.at(modalityIndex).at(listIndex) == "")
      {
        MITK_INFO << "Marked Index " << listIndex << " for removal.";
        indexRemoval.push_back(listIndex);
        break;
      }
    }
  }


  for (int listIndex = indexRemoval.size()-1 ; listIndex >= 0; --listIndex)
  {
    for (int i = 0 ; i < modalities; i++)
    {
      list.at(i).erase(list.at(i).begin()+indexRemoval.at(listIndex)) ;
    }
  }
  MITK_INFO << "Time Steps after sanitizing: " << list.at(0).size();
  return list;
}
