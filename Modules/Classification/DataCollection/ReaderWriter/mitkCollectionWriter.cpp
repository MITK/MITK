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

#include "mitkCollectionWriter.h"

#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include <mitkFiberBundleVtkReader.h>

#include "mitkImageCast.h"
#include "mitkTensorImage.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "mitkCoreObjectFactory.h"


#include <iostream>
#include <fstream>

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
const std::string FILEPATH = "filepath";
const std::string LINK = "link";


static std::string GetName(std::string fileName,std::string suffix, bool longName = false)
{
  fileName = QFileInfo(QString::fromStdString(fileName)).fileName().toStdString();
  if (longName)
    return fileName.substr(0,fileName.length() -suffix.length()-11); // 10 = date length
  else
    return fileName.substr(0,fileName.length() -suffix.length()-9); // 8 = date length
}

static std::string GetDate(std::string fileName,std::string suffix, bool longName = false)
{
  fileName = QFileInfo(QString::fromStdString(fileName)).fileName().toStdString();
  if (longName)
    fileName = fileName.substr(fileName.length() - suffix.length()-10,10); // 8 = date length
  else
    fileName = fileName.substr(fileName.length() - suffix.length()-8,8); // 8 = date length
  if (!longName)
  {
    fileName.insert(6,"-");
    fileName.insert(4,"-");
  }
  return fileName;
}




bool mitk::CollectionWriter::ExportCollectionToFolder(DataCollection *dataCollection, std::string xmlFile, std::vector<std::string> filter)
{
  // Quick and Dirty: Assumes three level DataCollection
  QDir fileName = QFileInfo(xmlFile.c_str()).absoluteDir();
  std::string outputFolder = fileName.path().toStdString() + QDir::separator().toLatin1();
  QDir baseFolder(outputFolder.c_str());
  baseFolder.mkpath(outputFolder.c_str());

  std::ofstream xmlFileStream;
  xmlFileStream.open (xmlFile.c_str());
  xmlFileStream << "<!-- MITK - DataCollection - File Version 1.0 --> \n";
  xmlFileStream << "<" << COLLECTION << " " << NAME << "=\"" << dataCollection->GetName() << "\" >\n";
  unsigned int subColId = 0;

  unsigned int dataId = 0;

  QDir dir(QString::fromStdString(outputFolder));
  for (size_t i = 0 ; i < dataCollection->Size(); ++i)
  {
    // Write Subcollection tag
    xmlFileStream << "  <" << SUBCOLLECTION <<  " " << NAME << "=\"" << dataCollection->IndexToName(i) << "\" " <<  FILEPATH << "=\"" << dataCollection->GetDataFilePath(i) << "\" id=\"Col" << subColId << "\" >\n";
    // Create Sub-Folder
    dir.mkpath(QString::fromStdString(dataCollection->IndexToName(i)));

    // Herein create data folders
    DataCollection* subCollections = dynamic_cast<DataCollection*> (dataCollection->GetData(i).GetPointer());
    if (subCollections == NULL)
    {
      MITK_ERROR<< "mitk::CollectionWriter::SaveCollectionToFolder: Container is illformed. Aborting";
      return false;
    }

    for (size_t d = 0; d < subCollections->Size(); d++ )
    {
      // Create Sub Paths
      QString subPath = QString::fromStdString(dataCollection->IndexToName(i))+"/"+QString::fromStdString(subCollections->IndexToName(d));
      dir.mkpath(subPath);
      xmlFileStream << "  <" << DATA <<  " " << NAME << "=\"" << subCollections->IndexToName(d) << "\" " <<  FILEPATH << "=\"" << subCollections->GetDataFilePath(d) << "\" id=\"Data" << dataId << "\" >\n";

      DataCollection* itemCollections = dynamic_cast<DataCollection*> (subCollections->GetData(d).GetPointer());
      if (itemCollections == NULL)
      {
        MITK_ERROR<< "mitk::CollectionWriter::SaveCollectionToFolder: Container is illformed. Aborting";
        return false;
      }

      for (size_t s = 0; s < itemCollections->Size(); s++)
      {
        if (filter.size() > 0)
        {
          bool isSelected = false;
          for (size_t f = 0; f < filter.size(); f++)
          {
            if (filter.at(f) == itemCollections->IndexToName(s) )
            {
              isSelected = true;
              break;
            }
          }
          if (isSelected == false)
            continue;
        }
        Image* image = dynamic_cast<Image*> (itemCollections->GetData(s).GetPointer());
        QString fileName = dir.path() + dir.separator() + subPath + dir.separator() +  QString::fromStdString(dataCollection->IndexToName(i)) + "_" + QString::fromStdString(subCollections->IndexToName(d)) + "_" + QString::fromStdString(itemCollections->IndexToName(s));
        try
        {
          if (itemCollections->IndexToName(s) == "DTI" || itemCollections->IndexToName(s) == "DTIFWE")
          {
            fileName += ".dti";
            itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
            io->SetFileType( itk::ImageIOBase::Binary );
            io->UseCompressionOn();
            TensorImage* tensorImage = dynamic_cast<TensorImage*> (image);
            MITK_INFO << "Pixeltype Tensor Image: " << tensorImage->GetPixelType().GetPixelTypeAsString();
            typedef itk::Image<itk::DiffusionTensor3D<TensorScalar>,3> ImageType;
            typedef itk::ImageFileWriter<ImageType> WriterType;
            WriterType::Pointer nrrdWriter = WriterType::New();
            ImageType::Pointer outimage = ImageType::New();
            CastToItkImage(tensorImage, outimage);
            nrrdWriter->SetInput( outimage );
            nrrdWriter->SetImageIO(io);
            nrrdWriter->SetFileName(fileName.toStdString());
            nrrdWriter->UseCompressionOn();
            nrrdWriter->Update();
          }
          else if (itemCollections->IndexToName(s) == "FIB")
          {
            fileName += ".fib";
            FiberBundle* fib = dynamic_cast<FiberBundle*> (itemCollections->GetData(s).GetPointer());
            CoreObjectFactory::FileWriterList fileWriters = CoreObjectFactory::GetInstance()->GetFileWriters();
            for (CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
            {
              if ( (*it)->CanWriteBaseDataType(fib) ) {
                (*it)->SetFileName( fileName.toStdString().c_str() );
                (*it)->DoWrite( fib );
              }
            }
          }
          else if (itemCollections->IndexToName(s) == "DWI")
          {
            fileName += ".dwi";
//            NrrdDiffusionImageWriter dwiwriter;
//            dwiwriter.SetInput( dynamic_cast<mitk::DiffusionImage<short>* > (image));
//            dwiwriter.SetOutputLocation( fileName.toStdString() );

            IOUtil::SaveImage(image,fileName.toStdString());
          }
          else
          {
            fileName += ".nrrd";
            Image::Pointer image = itemCollections->GetMitkImage(s).GetPointer();
            IOUtil::SaveImage(image,fileName.toStdString());
          }
        }
        catch( const std::exception& e)
        {
          MITK_ERROR << "Caught exception: " << e.what();
        }

        std::string relativeFilename = baseFolder.relativeFilePath(fileName).toStdString();
        xmlFileStream << "    <" << ITEM <<  " " << NAME << "=\"" <<itemCollections->IndexToName(s) << "\" " <<  FILEPATH << "=\""  << "\" " << LINK << "=\"" << relativeFilename << "\" />\n";
      }
      xmlFileStream << "  </" << DATA << ">\n";
      dataId++;
    }

    xmlFileStream << "  </" << SUBCOLLECTION << ">\n";
    subColId++;
  }
  xmlFileStream << "</" << COLLECTION << ">\n";
  xmlFileStream.flush();
  xmlFileStream.close();
  return true;
}

bool mitk::CollectionWriter::ExportCollectionToFolder(mitk::DataCollection *dataCollection, std::string xmlFile)
{
  std::vector<std::string> mods;
  return ExportCollectionToFolder(dataCollection,xmlFile, mods);
}

bool mitk::CollectionWriter::SaveCollection(mitk::DataCollection *dataCollection, std::vector<std::string> filter, std::string xmlFile)
{
  QDir origFilename = QFileInfo(dataCollection->GetXMLFile().c_str()).absoluteDir();
  QString originalFolder = origFilename.path() + QDir::separator();

  if (xmlFile == "")
    xmlFile = dataCollection->GetXMLFile();

  QDir fileName = QFileInfo(xmlFile.c_str()).absoluteDir();
  std::string outputFolder = fileName.path().toStdString() + QDir::separator().toLatin1();
  QDir baseFolder(outputFolder.c_str());

  std::ofstream xmlFileStream;
  xmlFileStream.open (xmlFile.c_str());
  xmlFileStream << "<!-- MITK - DataCollection - File Version 1.0 --> \n";
  xmlFileStream << "<" << COLLECTION << " " << NAME << "=\"" << dataCollection->GetName() << "\" >\n";
  unsigned int subColId = 0;

  unsigned int dataId = 0;

  QDir dir(QString::fromStdString(outputFolder));
  for (size_t i = 0 ; i < dataCollection->Size(); ++i)
  {
    // Write Subcollection tag
    xmlFileStream << "  <" << SUBCOLLECTION <<  " " << NAME << "=\"" << dataCollection->IndexToName(i) << "\" " <<  FILEPATH << "=\"" << dataCollection->GetDataFilePath(i) << "\" id=\"Col" << subColId << "\" >\n";
    // Create Sub-Folder
    dir.mkpath(QString::fromStdString(dataCollection->IndexToName(i)));

    // Herein create data folders
    DataCollection* subCollections = dynamic_cast<DataCollection*> (dataCollection->GetData(i).GetPointer());
    if (subCollections == NULL)
    {
      MITK_ERROR<< "mitk::CollectionWriter::SaveCollectionToFolder: Container is illformed. Aborting";
      return false;
    }

    for (size_t d = 0; d < subCollections->Size(); d++ )
    {
      // Create Sub Paths
      QString subPath = QString::fromStdString(dataCollection->IndexToName(i))+"/"+QString::fromStdString(subCollections->IndexToName(d));
      dir.mkpath(subPath);
      xmlFileStream << "  <" << DATA <<  " " << NAME << "=\"" << subCollections->IndexToName(d) << "\" " <<  FILEPATH << "=\"" << subCollections->GetDataFilePath(d) << "\" id=\"Data" << dataId << "\" >\n";

      DataCollection* itemCollections = dynamic_cast<DataCollection*> (subCollections->GetData(d).GetPointer());
      if (itemCollections == NULL)
      {
        MITK_ERROR<< "mitk::CollectionWriter::SaveCollectionToFolder: Container is illformed. Aborting";
        return false;
      }

      for (size_t s = 0; s < itemCollections->Size(); s++)
      {
        if (filter.size() > 0)
        {
          bool isSelected = false;
          for (size_t f = 0; f < filter.size(); f++)
          {
            if (filter.at(f) == itemCollections->IndexToName(s) )
            {
              isSelected = true;
              break;
            }
          }
          if (isSelected == false)
            continue;
        }
        Image* image = dynamic_cast<Image*> (itemCollections->GetData(s).GetPointer());
        QString fileName;
        bool fullName = false;
        if (itemCollections->GetDataFilePath(s) != "")
        {
          fileName = originalFolder +  QString::fromStdString(itemCollections->GetDataFilePath(s));
          fullName = true;
          MITK_INFO << "original path: " << itemCollections->GetDataFilePath(s) ;
        }
        else
          fileName = dir.path() + dir.separator() + subPath + dir.separator() +  QString::fromStdString(dataCollection->IndexToName(i)) + "_" + QString::fromStdString(subCollections->IndexToName(d)) + "_" + QString::fromStdString(itemCollections->IndexToName(s));

        try
        {
          if (itemCollections->IndexToName(s) == "DTI" || itemCollections->IndexToName(s) == "DTIFWE")
          {
            if (!fullName)
              fileName += ".dti";
            itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
            io->SetFileType( itk::ImageIOBase::Binary );
            io->UseCompressionOn();
            TensorImage* tensorImage = dynamic_cast<TensorImage*> (image);
            MITK_INFO << "Pixeltype Tensor Image: " << tensorImage->GetPixelType().GetPixelTypeAsString();
            typedef itk::Image<itk::DiffusionTensor3D<TensorScalar>,3> ImageType;
            typedef itk::ImageFileWriter<ImageType> WriterType;
            WriterType::Pointer nrrdWriter = WriterType::New();
            ImageType::Pointer outimage = ImageType::New();
            CastToItkImage(tensorImage, outimage);
            nrrdWriter->SetInput( outimage );
            nrrdWriter->SetImageIO(io);
            nrrdWriter->SetFileName(fileName.toStdString());
            nrrdWriter->UseCompressionOn();
            nrrdWriter->Update();
          }
          else if (itemCollections->IndexToName(s) == "FIB")
          {
            if (!fullName)
              fileName += ".fib";
            FiberBundle* fib = dynamic_cast<FiberBundle*> (itemCollections->GetData(s).GetPointer());
            CoreObjectFactory::FileWriterList fileWriters = CoreObjectFactory::GetInstance()->GetFileWriters();
            for (CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
            {
              if ( (*it)->CanWriteBaseDataType(fib) ) {
                (*it)->SetFileName( fileName.toStdString().c_str() );
                (*it)->DoWrite( fib );
              }
            }
          }
          else if (itemCollections->IndexToName(s) == "DWI")
          {
            if (!fullName)
              fileName += ".dwi";
            IOUtil::SaveImage(image,fileName.toStdString());
          }
          else
          {
            if (!fullName)
              fileName += ".nrrd";
            Image::Pointer image = itemCollections->GetMitkImage(s).GetPointer();
            IOUtil::SaveImage(image,fileName.toStdString());
          }
        }
        catch( const std::exception& e)
        {
          MITK_ERROR << "Caught exception: " << e.what();
        }

        std::string relativeFilename =baseFolder.relativeFilePath(fileName).toStdString();
        xmlFileStream << "    <" << ITEM <<  " " << NAME << "=\"" <<itemCollections->IndexToName(s) << "\" " <<  FILEPATH << "=\""  << "\" " << LINK << "=\"" << relativeFilename << "\" />\n";
      }
      xmlFileStream << "  </" << DATA << ">\n";
      dataId++;
    }

    xmlFileStream << "  </" << SUBCOLLECTION << ">\n";
    subColId++;
  }
  xmlFileStream << "</" << COLLECTION << ">\n";
  xmlFileStream.flush();
  xmlFileStream.close();
  return true;
}

bool mitk::CollectionWriter::FolderToXml(std::string folder, std::string collectionType, std::string xmlFile, std::vector<std::string> filter, std::vector<std::string> seriesNames)
{
  // 1) Parse for folders

  QDir parseDir;
  parseDir.setFilter( QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
  parseDir.setPath(QString::fromStdString(folder));

  QFileInfoList qFileList = parseDir.entryInfoList();


  std::ofstream xmlFileStream;
  xmlFileStream.open (xmlFile.c_str());
  xmlFileStream << "<!-- MITK - DataCollection - File Version 1.0 --> \n";
  xmlFileStream << "<" << COLLECTION << " " << NAME << "=\"" << "GEN" << "\" >\n";

  unsigned int dataId = 0;

  // now populate lists with files names, non-existing files will be marked with an empty string
  for (int i = 0; i < qFileList.size(); ++i)
  {
    // 2) For Each sub folder construct collectionType sub-folder
    std::string baseFolder = qFileList.at(i).absoluteFilePath().toStdString() + QDir::separator().toLatin1() + collectionType;

    MITK_INFO << "Processing : " << baseFolder;
    if (!QFileInfo(QString::fromStdString(baseFolder)).isDir())
    {
      MITK_WARN << "Not a valid folder, skipping.";
      continue;
    }

    // 3) Parse each sub folder and extend XML file
    // Parse folder and look up all data,
    // after sanitation only fully available groups are included (that is all suffixes are found)

    CollectionReader::FileListType fileList = CollectionReader::SanitizeFileList(CollectionReader::GenerateFileLists(baseFolder, filter,true));
    if (fileList.size() <= 0 || fileList.at(0).size() <= 0)
      continue;

    // Write Subcollection tag
    // try to extract date out of filename
    std::string name = GetName(fileList.at(0).at(0),filter.at(0));
    xmlFileStream << "  <" << SUBCOLLECTION <<  " " << NAME << "=\"" << name << "\" " <<  FILEPATH << "=\"\" id=\"Col" << i << "\" >\n";


    for (unsigned int k=0; k < fileList.at(0).size(); ++k) // all groups have the same amount of items, so looking at 0 is ok.
    {
      std::string strDate = GetDate(fileList.at(0).at(k),filter.at(0));
      xmlFileStream << "    <" << DATA <<  " " << NAME << "=\"" << strDate << "\" " <<  " id=\"Data" << dataId << "\" >\n";
      dataId++;
      for (unsigned int i=0; i< filter.size(); ++i)
      {
        std::string fileName = fileList.at(i).at(k);
        xmlFileStream << "      <" << ITEM <<  " " << NAME << "=\"" << seriesNames.at(i) << "\" " << LINK << "=\"" << fileName << "\" />\n";
      }
      xmlFileStream << "    </" << DATA << ">\n" ;
    }
    xmlFileStream << "  </" << SUBCOLLECTION << ">\n";
  }

  xmlFileStream << "</" << COLLECTION << ">\n";
  xmlFileStream.flush();
  xmlFileStream.close();

  return true;
}

bool mitk::CollectionWriter::SingleFolderToXml(std::string folder, std::string xmlFile, std::vector<std::string> filter, std::vector<std::string> seriesNames, bool longDate, int skipUntil, float months)
{
  std::ofstream xmlFileStream;
  xmlFileStream.open (xmlFile.c_str());
  xmlFileStream << "<!-- MITK - DataCollection - File Version 1.0 --> \n";
  xmlFileStream << "<" << COLLECTION << " " << NAME << "=\"" << "GEN" << "\" >\n";

  unsigned int dataId = 0;

  // 1)
  // Parse folder and look up all data,
  // after sanitation only fully available groups are included (that is all suffixes are found)

  CollectionReader::FileListType fileList = CollectionReader::SanitizeFileList(CollectionReader::GenerateFileLists(folder, filter,true));

  // Write Subcollection tag
  // try to extract date out of filename
  std::string name = GetName(fileList.at(0).at(0),filter.at(0),longDate);
  xmlFileStream << "  <" << SUBCOLLECTION <<  " " << NAME << "=\"" << name << "\" " <<  FILEPATH << "=\"\" id=\"Col" << 0 << "\" >\n";


  for (unsigned int k=skipUntil; k < fileList.at(0).size(); ++k) // all groups have the same amount of items, so looking at 0 is ok.
  {
    std::string strDate = GetDate(fileList.at(0).at(k),filter.at(0),true);
    xmlFileStream << "    <" << DATA <<  " " << NAME << "=\"" << strDate << "\" " <<  " id=\"Data" << dataId << "\" >\n";
    dataId++;
    for (unsigned int i=0; i< filter.size(); ++i)
    {
      std::string fileName = fileList.at(i).at(k);
      xmlFileStream << "      <" << ITEM <<  " " << NAME << "=\"" << seriesNames.at(i) << "\" " << LINK << "=\"" << fileName << "\" />\n";
    }

    //    size_t ind = GetIndexForinXMonths(fileList,months,k,filter);
    //    xmlFileStream << "      <" << ITEM <<  " " << NAME << "=\"TARGET\" " << LINK << "=\"" << fileList.at(filter.size()-1).at(ind) << "\" />\n";

    xmlFileStream << "    </" << DATA << ">\n" ;
    // check if target still exists for next step
    if (GetIndexForinXMonths(fileList,months,k+1,filter)== 0)
      break;
  }
  xmlFileStream << "  </" << SUBCOLLECTION << ">\n";

  xmlFileStream << "</" << COLLECTION << ">\n";
  xmlFileStream.flush();
  xmlFileStream.close();

  return true;
}

size_t mitk::CollectionWriter::GetIndexForinXMonths(mitk::CollectionReader::FileListType fileList,float months, size_t curIndex,std::vector<std::string> filter)
{
  std::string strDate0 = GetDate(fileList.at(0).at(curIndex),filter.at(0),true);

  int year0 =std::atoi(strDate0.substr(0,4).c_str());
  int month0 =std::atoi(strDate0.substr(5,2).c_str());
  int day0 = std::atoi(strDate0.substr(8,2).c_str());

  size_t bestIndex = 0;
  int bestFit = 1e5;

  for (size_t i=curIndex+1; i < fileList.at(0).size(); ++i)
  {
    std::string strDate = GetDate(fileList.at(0).at(i),filter.at(0),true);
    int year =std::atoi(strDate.substr(0,4).c_str());
    int month =std::atoi(strDate.substr(5,2).c_str());
    int day = std::atoi(strDate.substr(8,2).c_str());

    int fit = std::fabs((months * 30 ) - (((year-year0)*360) +((month-month0)*30) + (day-day0))); // days difference from x months
    if (fit < bestFit)
    {
      bestFit = fit;
      bestIndex = i;
    }
  }
  return bestIndex;
}
