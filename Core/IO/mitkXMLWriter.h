/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
  //## Function BeginNode() begins a XML node.
  //## The functions WriteProperty() are used to write XML attributes.
  //## Function EndNode() closes an open XML node tag.
  //## This class can also manage the writing of source files (path and filename operations).
  //##
  //## The XMLIO class manages the writing and reading of data.
  //## An example of how to start the XMLWriter can be found in the function Save() of the DataTree class.
  //## @ingroup IO
  class XMLWriter : public BaseXMLWriter {

    std::string m_Filename;
    std::string m_SubFolder;
    std::string m_SourceFileName;
    std::string m_FilenameAndSubFolder;
    std::string m_OriginPath;
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
    void WriteProperty( const std::string& key, const char* value ) const;

    /// Writes a XML attribute that datatype is a const std::string&. Parameter key specifies the name of the attribute. Parameter value represents the data of the attribute.
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

    /// sets the origin path of the source file
    void SetOriginPath(std::string originPath);

    /// returns false if file extension not exists
    bool IsFileExtension(std::string fileExtensionKey, std::string& fileName);

  protected:
    /// returns the relative path
    const std::string GetRelativePathAndFilename(std::string sourcePath);

    /// Checks whether the path has an ending slash. If there is no slash an ending slash is been added.
    std::string CheckLastSlash(std::string m_String);

  };
}
#endif
