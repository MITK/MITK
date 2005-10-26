#include "mitkXMLWriter.h"

#include <fstream>
#include <string>
#include <sstream>
#include <itksys/SystemTools.hxx>

namespace mitk {

std::string XMLWriter::m_ImageExtension = ".pic";

/**
 * Construktor
 */
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

/**
 * Constructor
 */
XMLWriter::XMLWriter( std::ostream& out, int space )
  : BaseXMLWriter( out, space ), m_FileCounter(0)
{
}

/**
 * Destructor
 */
XMLWriter::~XMLWriter() 
{
}

/**
	* Write bool Property
	*/
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
/**
	* Write bool Property
	*/
void XMLWriter::WriteProperty( const std::string& key, const mitk::Vector3D& value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}

/**
	* Write bool Property
	*/
void XMLWriter::WriteProperty( const std::string& key, const itk::Point<int,3> value ) const
{
  std::stringstream stream;;
  stream << value;
  BaseXMLWriter::WriteProperty( key, stream.str() );
}


/**
  * get the subfolder of the xml-File. 
  */
void XMLWriter::SetSubFolder( const char* subFolder )
{
  m_SubFolder = subFolder;
}

/**
  * get the subfolder of the xml-File. 
  */
const char* XMLWriter::GetSubFolder()
{
  return m_SubFolder.c_str();
}

/**
  * get an new unique FileName in the subdirectory of the xml-File
  */
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

/**
  * get an new unique FileName in the subdirectory of the xml-File
  */
const char* XMLWriter::GetNewFilenameAndSubFolder()
{
  static std::string newFilenameAndSubFolder = m_SubFolder;

  newFilenameAndSubFolder += GetNewFileName();
  return newFilenameAndSubFolder.c_str();
}

/**
 *
 */
void XMLWriter::SetImageExtension( const std::string& imageExtension )
{
  m_ImageExtension = imageExtension;
}

/**
 *
 */
const std::string XMLWriter::GetImageExtension()
{
  return m_ImageExtension;
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
