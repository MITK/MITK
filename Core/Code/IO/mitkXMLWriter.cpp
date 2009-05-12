/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkXMLWriter.h"

#include <fstream>
#include <string>
#include <sstream>
#include <itksys/SystemTools.hxx>

namespace mitk {

std::string XMLWriter::m_ImageExtension = ".pic";     // what is THIS doing HERE?


XMLWriter::XMLWriter( const char* filename, const char* subDirectory, int space )
: BaseXMLWriter( filename, space ), m_FileCounter(0)
{   
  if(filename != NULL)
    m_Filename = itksys::SystemTools::GetFilenameName(filename);
  else
    m_Filename = "";
  m_SubFolder = itksys::SystemTools::GetFilenamePath(filename);
  if(m_SubFolder.size() > 0)
    m_SubFolder += "/";
  if(subDirectory != NULL)
    m_SubFolder += subDirectory;
  if(m_SubFolder.size() > 0)
  {
    itksys::SystemTools::ConvertToUnixSlashes(m_SubFolder);
    m_SubFolder += "/";
  }
}


/*
XMLWriter::XMLWriter( std::ostringstream& out, int space )
: BaseXMLWriter( out, space ), m_FileCounter(0)
{

}
*/

XMLWriter::XMLWriter( const char* filename, int space )
  : BaseXMLWriter( filename, space ), m_FileCounter(0)
{
}


XMLWriter::~XMLWriter() 
{
}


void XMLWriter::SetSourceFileName( const char* sourceFileName)
{
  m_SourceFileName = sourceFileName;
}


void XMLWriter::SetSubFolder( const char* subFolder )
{
  m_SubFolder = CheckLastSlash(subFolder);
}


const char* XMLWriter::GetSubFolder()
{
  return m_SubFolder.c_str();
}


const char* XMLWriter::GetNewFileName()
{
  static std::string newFileName;
  char buffer[20];
  std::string::size_type pos = m_Filename.find('.');

  if ( pos == std::string::npos )
    newFileName = m_Filename;
  else
    newFileName = std::string( m_Filename, 0, pos );

  newFileName += '_';
  sprintf( buffer, "%d", ++m_FileCounter );
  newFileName += buffer;
  return newFileName.c_str();
}


const std::string XMLWriter::GetRelativePath()
{
  static std::string newFilenameAndSubFolder = m_SubFolder;

  if (m_SourceFileName != ""){
    if(!SaveSourceFiles())
      SetSubFolder( m_OriginPath.c_str() );
    m_FilenameAndSubFolder = m_SubFolder;
    //m_SourceFileName = itksys::SystemTools::GetFilenameWithoutExtension( m_SourceFileName );
    m_FilenameAndSubFolder += m_SourceFileName;
    return GetRelativePathAndFilename(m_FilenameAndSubFolder);
  }
  else{
    newFilenameAndSubFolder += GetNewFileName();
    return GetRelativePathAndFilename(newFilenameAndSubFolder);
  }
  m_SourceFileName = "";
}


void XMLWriter::SetImageExtension( const std::string& imageExtension )
{
  m_ImageExtension = imageExtension;
}


const std::string XMLWriter::GetImageExtension()
{
  return m_ImageExtension;
}


void XMLWriter::SetSaveSourceFiles(bool saveSourceFiles)
{
  m_SaveSourceFiles = saveSourceFiles;
}


bool XMLWriter::SaveSourceFiles()
{
  return m_SaveSourceFiles;
}

const std::string XMLWriter::GetRelativePathAndFilename(std::string sourcePath)
{ 
  // var declaration
  std::string relativePath;
  const std::string DirUp = "..";

  std::vector<std::string> xmlPathComponents;
  std::vector<std::string> sourcePathComponents;
  std::vector<std::string> relativePathComponents;

  std::string::size_type pos = 0;
  int xmlComps = 0;
  int sourceComps = 0;
  int pathDiffPos = -1;
  int directoriesUp = 0;
  //

  // get paths
  const std::string xmlFilePath = GetXMLPathAndFileName();
  const std::string sourceFilePath = sourcePath;

  // get number of elements of the xmlFilePath
  pos = xmlFilePath.find("/", pos);
  while(pos!=std::string::npos){
    ++xmlComps;
    pos = xmlFilePath.find("/", pos+1);
  }
  
  // get number of elements of the sourceFilePath
  pos = 0;
  pos = sourceFilePath.find("/", pos);
  while(pos!=std::string::npos){
    ++sourceComps;
    pos = sourceFilePath.find("/", pos+1);
  }

  // split paths into its components
  itksys::SystemTools::SplitPath(xmlFilePath.c_str(), xmlPathComponents);
  itksys::SystemTools::SplitPath(sourceFilePath.c_str(), sourcePathComponents);

  // the sourceFilePath contains only the file name
  // todo check wenn string leer!!!
  if(sourceComps==0){
    xmlPathComponents[xmlComps] = sourceFilePath;
    m_FilenameAndSubFolder = itksys::SystemTools::JoinPath(xmlPathComponents);
    return sourceFilePath;
  }

  // get the position of the component that differs in both paths  
  int counter = 0;
  while(counter<sourceComps){
    if(xmlPathComponents[counter]!=sourcePathComponents[counter]){
      pathDiffPos = counter;
      counter=sourceComps;
    }
    ++counter;
  }

  // sourceFilePath and xmlFilePath are equal
  if(pathDiffPos<0){
    // xmlFilePath is longer than sourceFilePath and the path components are equal
    if(xmlComps>sourceComps){
      int diffComps = xmlComps-sourceComps;
      relativePathComponents.resize(diffComps+2);
      for(int i=1; i<=diffComps; ++i){
        relativePathComponents[i] = DirUp;
      }
      relativePathComponents[diffComps+1] = sourcePathComponents[sourceComps];
      return itksys::SystemTools::JoinPath(relativePathComponents);
    }
    return sourcePathComponents[sourceComps];
  }
  // calculate how many "../" have to add
  if(pathDiffPos>=0)
    directoriesUp = (xmlComps - pathDiffPos);

  // setup of the relative path components
  relativePathComponents.resize(directoriesUp+(sourceComps-pathDiffPos)+2);

  for(int i=1; i<=directoriesUp; ++i){
    relativePathComponents[i] = DirUp;
  }

  int j = directoriesUp+1;
  for(int i=pathDiffPos; i<=sourceComps; ++i){
    relativePathComponents[j] = sourcePathComponents[i];
    ++j;
  }
  relativePath = itksys::SystemTools::JoinPath(relativePathComponents);

  // remove one "/" after ":"  necessary for windows when source file and xml file stored on different devices "://"->":/"
  if(pathDiffPos==0){
    std::string::size_type m_pos = relativePath.find(":");
      if(m_pos!=std::string::npos)
        relativePath.erase(m_pos+1, 1);
    return relativePath;
  }

  return relativePath;
}


const std::string XMLWriter::GetAbsolutePath()
{
  return m_FilenameAndSubFolder;
}


void XMLWriter::SetOriginPath(std::string originPath)
{
  m_OriginPath = CheckLastSlash(originPath);
}


std::string XMLWriter::CheckLastSlash(std::string m_String)
{
  if(m_String != ""){
    std::string::size_type lenght = m_String.length();
    if(m_String.substr(lenght-1, 1) != "/")
      m_String += "/";
  }
  return m_String;
}

bool XMLWriter::IsFileExtension(std::string fileExtensionKey, std::string& fileName)
{
  std::string::size_type extensionPos;
  extensionPos = fileName.find(fileExtensionKey);
  if(extensionPos==std::string::npos)
    return false;
  else{
    std::string::size_type lengthWithExtension;
    lengthWithExtension = extensionPos+fileExtensionKey.length();
    // removes e.g. ".gz" after the file extension
    if(lengthWithExtension < fileName.length())
      fileName.erase(lengthWithExtension, fileName.length()-lengthWithExtension);
    return true;
  }
}

} // namespace mitk
