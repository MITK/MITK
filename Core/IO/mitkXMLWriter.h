#ifndef MITK_XML_WRITER
#define MITK_XML_WRITER

#include "mitkBaseXMLWriter.h"
#include <mitkVector.h>
#include <itkRGBAPixel.h>
#include <itkRGBPixel.h>
#include <vtkSystemIncludes.h>

namespace mitk{

  /// Brief Class for writing XML nodes and XML attributes.
  ///
  /// Derived class of BaseXMLWriter.
  /// The data is stored with XML attributes.
  /// The functions WriteProperty() are used to write XML attributes. The first parameter specifies the name of the attribute. The second parameter holds the data of the attribute.
  /// Ingroup IO
  class XMLWriter : public BaseXMLWriter {

    std::string m_Filename;
    std::string m_SubFolder;
    int m_FileCounter;
    static std::string m_ImageExtension;

  public:

    /// constructor
    XMLWriter( const char* filename, const char* subDirectory, int space = 3);

    /// constructor
    XMLWriter( std::ostream& out, int space = 3 );

    /// destructor
    virtual ~XMLWriter();

    typedef itk::RGBAPixel< vtkFloatingPointType >  RGBAType;
    typedef itk::RGBPixel<vtkFloatingPointType> Color;

    /// writes a XML attribute that datatype is a const char*
    void WriteProperty( const std::string& key, const char* value ) const;

    /// writes a XML attribute that datatype is a const std::string&
    void WriteProperty( const std::string& key, const std::string& value ) const;

    /// writes a XML attribute that datatype is an integer
    void WriteProperty( const std::string& key, int value ) const;

    /// writes a XML attribute that datatype is a float
    void WriteProperty( const std::string& key, float value ) const;

    /// writes a XML attribute that datatype is a double
    void WriteProperty( const std::string& key, double value ) const;

    /// writes a XML attribute that datatype is a bool
    void WriteProperty( const std::string& key, bool value ) const;

    /// writes a XML attribute that datatype is a mitk::Point3D
    void WriteProperty( const std::string& key, const mitk::Point3D& value ) const;

    /// writes a XML attribute that datatype is a mitk::Point4D
    void WriteProperty( const std::string& key, const mitk::Point4D& value ) const;

    /// writes a XML attribute that datatype is a mitk::Vector3D
    void WriteProperty( const std::string& key, const mitk::Vector3D& value ) const;

    /// writes a XML attribute that datatype is an itk::Point<int,3>
    void WriteProperty( const std::string& key, const itk::Point<int,3> value ) const;

    /// writes a XML attribute that datatype is a RGBAType
    void WriteProperty( const std::string& key, RGBAType value ) const;

    /// writes a XML attribute that datatype is a Color (RGB)
    void WriteProperty( const std::string& key, Color value ) const;

   
    /// sets the subfolder of the XML file. 
    void SetSubFolder( const char* subFolder );

    /// returns the subfolder of the XML file. 
    const char* GetSubFolder();

   
    /// returns a new unique filename in the subdirectory of the XML file
    const char* GetNewFileName();

    
    /// returns a new unique filename in the subdirectory of the XML file
    const char* GetNewFilenameAndSubFolder();

    /// sets the image file extension (e.g. .pic, .mhd)
    // .pic is default
    void SetImageExtension( const std::string& imageExtension );

    /// returns the image file extension (e.g. .pic, .mhd)
    const std::string GetImageExtension();
  };
}
#endif
