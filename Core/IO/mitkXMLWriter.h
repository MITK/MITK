#ifndef MITK_XML_WRITER
#define MITK_XML_WRITER

#include "mitkBaseXMLWriter.h"
#include <mitkVector.h>

namespace mitk{

  class XMLWriter : public BaseXMLWriter {

    std::string m_Filename;
    std::string m_SubFolder;
    int m_FileCounter;
    static std::string m_ImageExtension;

	public:

		/**
		 * Construktor
		 */
		XMLWriter( const char* filename, const char* subDirectory, int space = 3);

		/**
		 * Construktor
		 */
		XMLWriter( std::ostream& out, int space = 3 );

		/**
		 * Destruktor
		 */
		virtual ~XMLWriter();

    void WriteProperty( const std::string& key, const char* value ) const;
    void WriteProperty( const std::string& key, const std::string& value ) const;
    void WriteProperty( const std::string& key, int value ) const;
    void WriteProperty( const std::string& key, float value ) const;
    void WriteProperty( const std::string& key, double value ) const;
    void WriteProperty( const std::string& key, bool value ) const;

		/**
		 * Write bool Property
		 */
    void WriteProperty( const std::string& key, const mitk::Point3D& value ) const;
    void WriteProperty( const std::string& key, const mitk::Point4D& value ) const;

		/**
		 * Write bool Property
		 */
    void WriteProperty( const std::string& key, const mitk::Vector3D& value ) const;

		/**
		 * Write bool Property
		 */
    void WriteProperty( const std::string& key, const itk::Point<int,3> value ) const;

    /**
     * get the subfolder of the xml-File. 
     */
    void SetSubFolder( const char* subFolder );

    /**
     * get the subfolder of the xml-File. 
     */
    const char* GetSubFolder();

    /*
     * get an new unique FileName in the subdirectory of the xml-File
     */
    const char* GetNewFileName();

    /*
     * get an new unique FileName in the subdirectory of the xml-File
     */
    const char* GetNewFilenameAndSubFolder();

    void SetImageExtension( const std::string& imageExtension );

    const std::string GetImageExtension();
	};
}
#endif
