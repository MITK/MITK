#ifndef MITK_XML_WRITER
#define MITK_XML_WRITER

#include "mitkBaseXMLWriter.h"
#include <mitkVector.h>
#include <itkRGBAPixel.h>
#include <itkRGBPixel.h>
#include <vtkSystemIncludes.h>

namespace mitk{

//##Documentation
//## @brief derived class of BaseXMLWriter.
//##
//## Note - the functions WriteProperty() are used to write XML attributes.

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

    typedef itk::RGBAPixel< vtkFloatingPointType >  RGBAType;
    typedef itk::RGBPixel<vtkFloatingPointType> Color;

    void WriteProperty( const std::string& key, const char* value ) const;
    void WriteProperty( const std::string& key, const std::string& value ) const;
    void WriteProperty( const std::string& key, int value ) const;
    void WriteProperty( const std::string& key, float value ) const;
    void WriteProperty( const std::string& key, double value ) const;
    void WriteProperty( const std::string& key, bool value ) const;

    /**
     * Writes a mitk::Point3D XML attribute
     */
    void WriteProperty( const std::string& key, const mitk::Point3D& value ) const;

    /**
     * Writes a mitk::Point4D XML attribute
     */
    void WriteProperty( const std::string& key, const mitk::Point4D& value ) const;

    /**
     * Writes a mitk::Vector3D XML attribute
     */
    void WriteProperty( const std::string& key, const mitk::Vector3D& value ) const;

    /**
     * Writes a itk::Point<int,3> XML attribute
     */
    void WriteProperty( const std::string& key, const itk::Point<int,3> value ) const;

    /**
     * Writes a RGBAType XML attribute
     */
    void WriteProperty( const std::string& key, RGBAType value ) const;

    /**
     * Writes a RGBType XML attribute
     */
    void WriteProperty( const std::string& key, Color value ) const;

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
