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
#include "Poco/Zip/Compress.h"
#include "Poco/Path.h"
#include "Poco/File.h"

#include "mitkNavigationToolStorageSerializer.h"
#include "mitkNavigationToolWriter.h"
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkSceneIO.h>
#include <mitkStandardFileLocations.h>

#include <sstream>



mitk::NavigationToolStorageSerializer::NavigationToolStorageSerializer()
{
  //create temp directory
  mitk::UIDGenerator myUIDGen = mitk::UIDGenerator("",16);
  m_tempDirectory = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() + "tempNavigationToolSerializer_" + myUIDGen.GetUID();
  Poco::File myFile(m_tempDirectory);
  myFile.createDirectory();
}

mitk::NavigationToolStorageSerializer::~NavigationToolStorageSerializer()
{
 //remove temp directory
 Poco::File myFile(m_tempDirectory);
 try
  {
  if (myFile.exists()) myFile.remove(true);
  }
 catch(...)
  {
   MITK_ERROR << "Can't remove temp directory " << m_tempDirectory << "!";
  }
}

bool mitk::NavigationToolStorageSerializer::Serialize(std::string filename, mitk::NavigationToolStorage::Pointer storage)
{
  //save every tool to temp directory
  mitk::NavigationToolWriter::Pointer myToolWriter = mitk::NavigationToolWriter::New();
  for(int i=0; i<storage->GetToolCount();i++)
  {
    std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
    if (!myToolWriter->DoWrite(fileName,storage->GetTool(i)))
      {
        mitkThrowException(mitk::IGTIOException) << "Could not write tool to tempory directory: " << filename;
      }
  }
  //add all files to zip archive
  std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out);
  if (!file.good()) //test if the zip archive is ready for writing
  {
    //first: clean up
    for (int i=0; i<storage->GetToolCount();i++)
    {
      std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
      std::remove(fileName.c_str());
    }
    //then: throw an exception
    mitkThrowException(mitk::IGTIOException) << "Could not open a file for writing: " << filename;
  }
  Poco::Zip::Compress zipper( file, true );
  for (int i=0; i<storage->GetToolCount();i++)
  {
    std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
    zipper.addFile(fileName,myToolWriter->GetFileWithoutPath(fileName));
    std::remove(fileName.c_str()); //delete file
  }
 zipper.close();
 file.close();

 return true;
 }

std::string mitk::NavigationToolStorageSerializer::convertIntToString(int i)
 {
  std::string s;
  std::stringstream out;
  out << i;
  s = out.str();
  return s;
 }
