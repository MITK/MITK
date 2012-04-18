/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//Poco headers
#include "Poco/Zip/Decompress.h"
#include "Poco/Path.h"
#include "Poco/File.h"

#include "mitkNavigationToolStorageDeserializer.h"
#include <mitkSceneIO.h>
#include <mitkStandardFileLocations.h>
#include "mitkNavigationToolReader.h"

//POCO
#include <Poco/Exception.h>

mitk::NavigationToolStorageDeserializer::NavigationToolStorageDeserializer(mitk::DataStorage::Pointer dataStorage)
  {
  m_DataStorage = dataStorage;
  //create temp directory for this reader
  m_tempDirectory = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() + "tempNavigationToolDeserializer";
  Poco::File myFile(m_tempDirectory);  
  myFile.createDirectory();
  }

mitk::NavigationToolStorageDeserializer::~NavigationToolStorageDeserializer()
  {
  //remove temp directory
  Poco::File myFile(m_tempDirectory);
  try
    {
    if (myFile.exists()) myFile.remove();
    }
  catch(...)
    {
    MITK_ERROR << "Can't remove temp directory " << m_tempDirectory << "!";
    }
  }

mitk::NavigationToolStorage::Pointer mitk::NavigationToolStorageDeserializer::Deserialize(std::string filename)
  {
  //decomress zip file into temporary directory
  decomressFiles(filename,m_tempDirectory);
  
  //now read all files and convert them to navigation tools
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New(m_DataStorage);
  bool cont = true;
  for (int i=0; cont==true; i++)
    {
    std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
    mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
    mitk::NavigationTool::Pointer readTool = myReader->DoRead(fileName);
    if (readTool.IsNull()) cont = false;
    else returnValue->AddTool(readTool);
    //delete file
    std::remove(fileName.c_str());
    }
  return returnValue;
  }

std::string mitk::NavigationToolStorageDeserializer::convertIntToString(int i)
  {
  std::string s;
  std::stringstream out;
  out << i;
  s = out.str();
  return s;
  }

bool mitk::NavigationToolStorageDeserializer::decomressFiles(std::string filename,std::string path)
  {
  std::ifstream file( filename.c_str(), std::ios::binary );
  if (!file.good())
    {
    m_ErrorMessage = "Cannot open '" + filename + "' for reading";
    return false;
    }
  Poco::Zip::Decompress unzipper( file, Poco::Path( path ) );
  unzipper.decompressAllFiles();
  file.close();
  return true;
  }