#include "mitkXMLWriter.h"

#include <fstream>
#include <string>
#include <stack>
#include <stdio.h>

namespace mitk {

/**
 * Construktor
 */
XMLWriter::XMLWriter( const char* filename, int space )
:m_Out(NULL), m_Increase(0), m_Space(space), m_NodeCount(0) , m_File(true), m_FirstNode(true), m_NodeIsClosed(true), m_NewNode(false), 
 m_Filename( filename ), m_SubFolder(), m_FileCounter(0)
{		
	m_Out = new std::ofstream( filename );			
}

/**
 * Construktor
 */
XMLWriter::XMLWriter( std::ostream& out, int space )
:m_Out(&out), m_Increase(0), m_Space(space), m_NodeCount(0), m_File(false), m_FirstNode(true), m_NodeIsClosed(true), m_NewNode(false),
 m_Filename(), m_SubFolder(), m_FileCounter(0)
{}

/**
 * Destruktor
 */
XMLWriter::~XMLWriter() 
{
	if ( m_File ) 
	{
		std::ofstream* file = static_cast<std::ofstream*>(m_Out);
		file->close();
		delete file;
	}
}

/*
 *
 */
void XMLWriter::BeginNode( const char* name ) 
{		
	if ( m_FirstNode ) 
	{
		*m_Out << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
		m_FirstNode = false;
	}

	if ( !m_NodeIsClosed ) 
	{
		*m_Out << ">\n";
		m_NodeIsClosed = true;
	}

	int steps = m_Space * m_Increase;

	for(int i=0; i < steps; i++) 
		*m_Out << ' '; 

	*m_Out << '<' << name << ' ';

	m_NodeIsClosed = false;
	m_NewNode = true;
	m_Stack.push( name );
	m_Increase++;
	m_NodeCount++;
}

/**
 *
 */
void XMLWriter::EndNode( )
{			
	m_Increase--;

	if ( m_NewNode )
		*m_Out << "/>\n";
	else 
	{
		int steps = m_Space * m_Increase;

		for(int i=0; i < steps; i++) 
			*m_Out << ' '; 
		*m_Out << "</" << m_Stack.top() << ">\n";				
	}

	m_Stack.pop();
	m_NodeIsClosed = true;
	m_NewNode = false;
}

/**
 * Write Property
 */
void XMLWriter::WriteProperty( const char* key, const char* value ) const
{
	*m_Out << ConvertString( key );
	*m_Out << "=\"" << ConvertString( value ) << "\" ";
}

/**
 * Write string Property
 */
void XMLWriter::WriteProperty( const char* key, const std::string& value ) const 
{
	*m_Out << ConvertString( key );
	*m_Out << "=\"" << ConvertString( value.c_str() ) << "\" ";
}


/**
 * Write int Property
 */
void XMLWriter::WriteProperty( const char* key, int value ) const 
{
	*m_Out << ConvertString( key );
	*m_Out << "=\"" << value << "\" ";
}

/**
 * Write float Property
 */
void XMLWriter::WriteProperty( const char* key, float value ) const
{
	*m_Out << ConvertString( key );
	*m_Out << "=\"" << value << "\" ";
}

/**
 * Write double Property
 */
void XMLWriter::WriteProperty( const char* key, double value ) const
{
	*m_Out << ConvertString( key );
	*m_Out << "=\"" << value << "\" ";
}

/**
	* Write bool Property
	*/
void XMLWriter::WriteProperty( const char* key, bool value ) const
{
	*m_Out << ConvertString( key );

  if ( value == true )
	  *m_Out << "=\"" << "TRUE\" ";
  else
    *m_Out << "=\"" << "FALSE\" ";
}

/**
	* Write comment
	*/
void XMLWriter::WriteComment( const char* text ) 
{
	if ( m_FirstNode ) 
	{
		*m_Out <<"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
		m_FirstNode = false;
	}

	int steps = m_Space * m_Increase;

	for(int i=0; i < steps; i++) 
		*m_Out << ' '; 

	*m_Out << "<!-- " << ConvertString( text ) <<  " -->\n";
}
	
/**
 * retun the current deph
 */
int XMLWriter::GetCurrentDeph() const
{
	return m_Stack.size();
}

/**
 *
 */
int XMLWriter::GetNodeCount() const
{
	return m_NodeCount;
}

/**
 * Get the space
 */
int XMLWriter::GetSpace() const
{
	return m_Space;
}

/**
 *
 */
void XMLWriter::SetSpace( int space )
{
	m_Space = space;
}

/**
 * replace char < and > through { and }
 */
const char* XMLWriter::ConvertString( const char* string ) const
{
	static std::char_traits<char>::char_type buffer[255];
	int length = std::char_traits<char>::length( string );
	std::char_traits<char>::copy ( buffer, string, length + 1 );
	const char* pos = buffer;
	
	while ( pos != NULL ) 
	{
		pos = std::char_traits<char>::find ( buffer, length , '<');

		if( pos != NULL )
			*(const_cast<char*>(pos)) = '{';
	}

	pos = buffer;
	
	while ( pos != NULL ) 
	{
		pos = std::char_traits<char>::find ( buffer, length , '>');

		if( pos != NULL )
			*(const_cast<char*>(pos)) = '}';
	}

	return buffer;
}

/**
  * get the subfolder of the xml-File. 
  */
void XMLWriter::setSubFolder( const char* subFolder )
{
  m_SubFolder = subFolder;
}

/**
  * get the subfolder of the xml-File. 
  */
const char* XMLWriter::getSubFolder()
{
  return m_SubFolder.c_str();
}

/*
  * get an new unique FileName in the subdirectory of the xml-File
  */
const char* XMLWriter::getNewFileName()
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

/*
  * get an new unique FileName in the subdirectory of the xml-File
  */
const char* XMLWriter::getNewFilenameAndSubFolder()
{
  static std::string newFilenameAndSubFolder;
  newFilenameAndSubFolder = m_SubFolder;

  std::string::size_type pos = m_SubFolder.find('\\');

  if ( pos != std::string::npos )
    newFilenameAndSubFolder += '\\';
  else
    newFilenameAndSubFolder += '/';

  newFilenameAndSubFolder += getNewFileName();
  return newFilenameAndSubFolder.c_str();
}

} // namespace mitk

