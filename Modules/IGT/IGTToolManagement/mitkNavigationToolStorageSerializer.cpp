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
#include <mitkStandaloneDataStorage.h>
#include <mitkSceneIO.h>
#include <mitkStandardFileLocations.h>

#include <sstream>
//for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

mitk::NavigationToolStorageSerializer::NavigationToolStorageSerializer()
{
  //create temp directory
  m_tempDirectory = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() + "tempNavigationToolSerializer";
  Poco::File myFile(m_tempDirectory);  
  myFile.createDirectory();
}

mitk::NavigationToolStorageSerializer::~NavigationToolStorageSerializer()
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

bool mitk::NavigationToolStorageSerializer::Serialize(std::string filename, mitk::NavigationToolStorage::Pointer storage)
{
  //save every tool to temp directory
  mitk::NavigationToolWriter::Pointer myToolWriter = mitk::NavigationToolWriter::New();
  for(int i=0; i<storage->GetToolCount();i++)
 {   
  std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
  if (!myToolWriter->DoWrite(fileName,storage->GetTool(i))) return false;
 }
  //add all files to zip archive
  std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out);
  if (!file.good())
 {
   //Exception if cannot open a zip file for writing
   mitkThrowException(mitk::IGTException)<<"Could not open a zip file for writing: '" + filename + "'";
   for (int i=0; i<storage->GetToolCount();i++)
 {
   std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
   std::remove(fileName.c_str());
 }
    return false;
 }
  Poco::Zip::Compress zipper( file, true );
  for (int i=0; i<storage->GetToolCount();i++)
 {
   std::string fileName = m_tempDirectory + Poco::Path::separator() + "NavigationTool" + convertIntToString(i) + ".tool";
   zipper.addFile(fileName,myToolWriter->GetFileWithoutPath(fileName));
   //delete file:
   std::remove(fileName.c_str());
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
