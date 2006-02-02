#ifndef MITK_XML_WRITER
#define MITK_XML_WRITER

#include "mitkBaseXMLWriter.h"
#include <mitkVector.h>
#include <itkRGBAPixel.h>
#include <itkRGBPixel.h>
#include <vtkSystemIncludes.h>

namespace mitk{

  //##Documentation
  //## @brief XMLWriter class for writing XML nodes and XML attributes.
  //##
  //## Derived class of BaseXMLWriter.
  //## The data is stored with XML attributes.
  //## Function BeginNode() writes a XML node.
  //## The functions WriteProperty() are used to write XML attributes.
  //## Function EndNode() closes an open XML node tag.
  //## @ingroup IO
  class XMLWriter : public BaseXMLWriter {

    std::string m_Filename;
    std::string m_SubFolder;
    std::string m_SourceFileName;
    std::string m_FilenameAndSubFolder;
    int m_FileCounter;
    bool m_SaveSourceFiles;
    bool isNotPath;
    static std::string m_ImageExtension;


  public:

    /// constructor
    XMLWriter( const char* filename, const char* subDirectory, int space = 3);

    /// constructor
    //XMLWriter( std::ostream& out, int space = 3 );

    /// constructor
    XMLWriter( const char* filename, int space = 3 );

    /// destructor
    virtual ~XMLWriter();

    typedef itk::RGBAPixel< vtkFloatingPointType >  RGBAType;
    typedef itk::RGBPixel<vtkFloatingPointType> Color;

    /// writes a XML attribute that datatype is a const char*
    void WriteProperty( const std::string& key, const char* value );

    /// Writes a XML attribute that datatype is a const std::string&. Parameter key specifies the name of the attribute. Parameter value represents the data of the attribute.
    void WriteProperty( const std::string& key, const std::string& value );

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

    /// sets the filename of a source file
    void SetSourceFileName( const char* sourceFileName);
   
    /// sets the subfolder of the source files
    void SetSubFolder( const char* subFolder );

    /// returns the subfolder of the source files 
    const char* GetSubFolder();
   
    /// returns a new unique filename in the subdirectory of the source file
    const char* GetNewFileName();
    
    /// Returns the relative path (subfolder + filename) of the source file. The path of the source file is relative to the XML file.
    const std::string GetRelativePath();

    /// Returns the absolute path (subfolder + filename) of the source file. 
    const std::string GetAbsolutePath();

    /// sets the image file extension (e.g. .pic, .mhd)
    // .pic is default
    void SetImageExtension( const std::string& imageExtension );

    /// returns the image file extension (e.g. .pic, .mhd)
    const std::string GetImageExtension();

    /// sets to save the source files
    void SetSaveSourceFiles(bool saveSourceFiles);

    /// checks whether the source files will be written 
    bool SaveSourceFiles();

  protected:
    /// returns the relative path
    const std::string GetRelativePathAndFilename(std::string sourcePath);

  };
}
#endif
