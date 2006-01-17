#include "mitkXMLWriter.h"

#include <fstream>
#include <string>
#include <sstream>
#include <itksys/SystemTools.hxx>

namespace mitk {

std::string XMLWriter::m_ImageExtension = ".pic";


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


//XMLWriter::XMLWriter( std::ostream& out, int space )
//  : BaseXMLWriter( out, space ), m_FileCounter(0)
//{
//}


XMLWriter::XMLWriter( const char* filename, int space )
  : BaseXMLWriter( filename, space ), m_FileCounter(0)
{
}


XMLWriter::~XMLWriter() 
{
}


void XMLWriter::WriteProperty( const std::string& key, const mitk::Point3D& value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}


void XMLWriter::WriteProperty( const std::string& key, const mitk::Point4D& value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}


void XMLWriter::WriteProperty( const std::string& key, const mitk::Vector3D& value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}

void XMLWriter::WriteProperty( const std::string& key, const itk::Point<int,3> value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}


void XMLWriter::WriteProperty( const std::string& key, RGBAType value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}


void XMLWriter::WriteProperty( const std::string& key, Color value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}


void XMLWriter::SetSourceFileName( const char* sourceFileName)
{
  m_SourceFileName = sourceFileName;
}


void XMLWriter::SetSubFolder( const char* subFolder )
{
  //m_SubFolder = GetRelativePath(subFolder);
  m_SubFolder = subFolder;  
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
    m_FilenameAndSubFolder = m_SubFolder;
    m_FilenameAndSubFolder += m_SourceFileName;
    return GetRelativePath(m_FilenameAndSubFolder);
  }
  else{
    newFilenameAndSubFolder += GetNewFileName();
    return GetRelativePath(newFilenameAndSubFolder);
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

const std::string XMLWriter::GetRelativePath(std::string sourcePath)
{ 
  // var declaration
  std::string relativePath;
  const std::string DirUp = "..";

  std::vector<std::string> xmlPathComponents;
  std::vector<std::string> sourcePathComponents;
  std::vector<std::string> relativePathComponents;

  unsigned int pos = 0;
  int xmlComps = 0;
  int sourceComps = 0;
  int pathDiffPos = -1;
  int directoriesUp = 0;
  //

  // get paths
  const std::string xmlFilePath = GetXMLFileName();
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

  // the sourceFilePath contains only the file name 
  if(sourceComps==0){
    return sourceFilePath;
  }

  // split paths into its components
  itksys::SystemTools::SplitPath(xmlFilePath.c_str(), xmlPathComponents);
  itksys::SystemTools::SplitPath(sourceFilePath.c_str(), sourcePathComponents);

  // get the position of the component that differs in both paths  
  int counter = 0;
  while(counter<sourceComps){
    if(xmlPathComponents[counter]!=sourcePathComponents[counter]){
      pathDiffPos = counter;
      counter=sourceComps;
    }
    ++counter;
  }

  // sourceFilePath and xmlFilePath are equal or consist different devices
  if(pathDiffPos<=0)
    return sourceFilePath;

  // calculate how many "../" have to add
  if(pathDiffPos>0)
    directoriesUp = (xmlComps - pathDiffPos)+1;

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

  return relativePath;
}


const std::string XMLWriter::GetAbsolutePath()
{
  return m_FilenameAndSubFolder;
}

void XMLWriter::WriteProperty( const std::string& key, const char* value ) const
{ XMLWriter::BaseXMLWriter::WriteProperty( key, value ); }
void XMLWriter::WriteProperty( const std::string& key, const std::string& value ) const
{ XMLWriter::BaseXMLWriter::WriteProperty( key, value ); }
void XMLWriter::WriteProperty( const std::string& key, int value ) const
{ XMLWriter::BaseXMLWriter::WriteProperty( key, value ); }
void XMLWriter::WriteProperty( const std::string& key, float value ) const
{ XMLWriter::BaseXMLWriter::WriteProperty( key, value ); }
void XMLWriter::WriteProperty( const std::string& key, double value ) const
{ XMLWriter::BaseXMLWriter::WriteProperty( key, value ); }
void XMLWriter::WriteProperty( const std::string& key, bool value ) const
{ XMLWriter::BaseXMLWriter::WriteProperty( key, value ); }

} // namespace mitk
