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
#include "Poco/Zip/Compress.h"
#include "Poco/Path.h"
#include "Poco/File.h"

#include "mitkNavigationToolStorageSerializer.h"
#include "mitkNavigationToolWriter.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkSceneIO.h>
#include <mitkStandardFileLocations.h>

#include <sstream>

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
  if (myFile.exists()) myFile.remove();
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
    m_ErrorMessage = "Could not open a zip file for writing: '" + filename + "'";
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
