/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkXMLSerializable_h
#define mitkXMLSerializable_h

#include <string>
#include <itkObject.h>
#include <itkMacro.h>
#include <MitkCameraCalibrationExports.h>

namespace tinyxml2
{
  class XMLElement;
}

namespace mitk
{
  ///
  /// \brief interface for all classes able to write themselves to XML files
  ///
  class MITKCAMERACALIBRATION_EXPORT XMLSerializable
  {
  public:
    ///
    /// value of the special tag for file references
    /// if this is attribute is found the class
    /// will be loaded from the file in the attributes value
    ///
    static const std::string FILE_REFERENCE_ATTRIBUTE_NAME;
    ///
    /// the name of the root node that is created when the element is saved
    ///
    static const std::string ROOT_NAME;
    ///
    /// the tag value will be equals to the class name (function
    /// implemented by the itkTypeMacro)
    ///
    virtual const char* GetNameOfClass() const = 0;

    ///
    /// write your values here to elem
    ///
    virtual void ToXML(tinyxml2::XMLElement* elem) const = 0;
    ///
    /// read your values here from elem
    ///
    virtual void FromXML(const tinyxml2::XMLElement* elem) = 0;

    ///
    /// tries to write the xml data obtained in ToXML() to file
    ///
    virtual void ToXMLFile(const std::string& file
                           , const std::string& elemName="");

    ///
    /// loads the XML file and calls FromXML()
    /// takes the first child of the document
    /// if this root node value is not equal to GetNameOfClass()
    /// the method will try to find the first children of the root
    /// node with the value of GetNameOfClass()
    /// if elemName is not empty then this value will be used instead
    /// of GetNameOfClass()
    /// if this node is found it will check if an attribute named
    /// FILE_REFERENCE_ATTRIBUTE_NAME is found: in this case
    /// the method calls itself with this attributes value as parameter
    ///
    virtual void FromXMLFile(const std::string& file
                             , const std::string& elemName="");

    ///
    /// \see m_XMLFileName
    ///
    std::string GetXMLFileName() const;

  private:
    ///
    /// saves the xmlfile name set for this serializable ( in FromXMLFile() )
    ///
    std::string m_XMLFileName;
  };
}

#endif
