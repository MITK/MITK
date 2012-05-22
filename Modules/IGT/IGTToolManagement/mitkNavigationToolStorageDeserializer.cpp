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
  bool success = false;
  
  //decomress zip file into temporary directory
  success = decomressFiles(filename,m_tempDirectory);
  
  //currently returns an empty storage in case of an error. TODO when exception handling is availiable in MITK: Throw an exception?
  if (!success) {return mitk::NavigationToolStorage::New();} 
  
  //now read all files and convert them to navigation tools
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New(m_DataStorage);
  bool cont = true;
  int i;
  for (i=0; cont==true; i++)
    {
    std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
    mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
    mitk::NavigationTool::Pointer readTool = myReader->DoRead(fileName);
    if (readTool.IsNull()) cont = false;
    else returnValue->AddTool(readTool);
    //delete file
    std::remove(fileName.c_str());
    }
  if(i==1)
    {
    m_ErrorMessage = "Error: did not find any tool. \n Is this a tool storage file?";
    MITK_ERROR << "Error: did not find any tool. Is this a tool storage file?";
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
  try
    {
    Poco::Zip::Decompress unzipper( file, Poco::Path( path ) );
    unzipper.decompressAllFiles();
    file.close();
    }
  catch(Poco::IllegalStateException e) //temporary solution: replace this by defined exception handling later!
    {
    m_ErrorMessage = "Error: wrong file format! \n (please only load tool storage files)";
    MITK_ERROR << "Error: wrong file format! (please only load tool storage files)";
    return false;
    }

  return true;
  }