/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationToolStorageSerializer.h"
#include "mitkNavigationToolWriter.h"
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkSceneIO.h>
#include <mitkIOUtil.h>

// Poco
#include <Poco/Zip/Compress.h>

//ITKsys
#include <itksys/SystemTools.hxx>

#include <sstream>

mitk::NavigationToolStorageSerializer::NavigationToolStorageSerializer()
{
  //create temp directory
  m_tempDirectory = mitk::IOUtil::CreateTemporaryDirectory("NavigationToolStorageSerializerTmp_XXXXXX");
}

mitk::NavigationToolStorageSerializer::~NavigationToolStorageSerializer()
{
 //remove temp directory

 try
  {
    if (itksys::SystemTools::FileExists(m_tempDirectory.c_str())) std::remove(m_tempDirectory.c_str());
  }
 catch(...)
  {
  MITK_ERROR << "Can't remove temp directory " << m_tempDirectory << "!";
  }
}

void mitk::NavigationToolStorageSerializer::Serialize(const std::string& filename, mitk::NavigationToolStorage::Pointer storage)
{
  //save every tool to temp directory
  mitk::NavigationToolWriter::Pointer myToolWriter = mitk::NavigationToolWriter::New();
  for(unsigned int i=0; i<storage->GetToolCount();i++)
  {
    std::string tempFileName = m_tempDirectory + mitk::IOUtil::GetDirectorySeparator() + "NavigationTool" + convertIntToString(i) + ".tool";
    if (!myToolWriter->DoWrite(tempFileName,storage->GetTool(i)))
      {
        mitkThrowException(mitk::IGTIOException) << "Could not write tool to tempory directory: " << tempFileName;
      }
  }
  //add all files to zip archive
  std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out);
  if (!file.good()) //test if the zip archive is ready for writing
  {
    //first: clean up
    for (unsigned int i=0; i<storage->GetToolCount();i++)
    {
      std::string tempFileName = m_tempDirectory + mitk::IOUtil::GetDirectorySeparator() + "NavigationTool" + convertIntToString(i) + ".tool";
      std::remove(tempFileName.c_str());
    }
    //then: throw an exception
    mitkThrowException(mitk::IGTIOException) << "Could not open a file for writing: " << filename;
  }
  Poco::Zip::Compress zipper( file, true );
  for (unsigned int i=0; i<storage->GetToolCount();i++)
  {
    std::string fileName = m_tempDirectory + mitk::IOUtil::GetDirectorySeparator() + "NavigationTool" + convertIntToString(i) + ".tool";
    zipper.addFile(fileName, myToolWriter->GetFileWithoutPath(fileName));
    std::remove(fileName.c_str()); //delete file
  }
 zipper.close();
 file.close();
}

std::string mitk::NavigationToolStorageSerializer::convertIntToString(int i)
 {
  std::string s;
  std::stringstream out;
  out << i;
  s = out.str();
  return s;
 }
