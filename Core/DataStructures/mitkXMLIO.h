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

#ifndef XMLIO_H_HEADER_INCLUDED
#define XMLIO_H_HEADER_INCLUDED

#include <string>

namespace mitk {

  class XMLWriter;
  class XMLReader;

  //##Documentation
  //## @brief XMLIO class for writing mitk classes as a XML node to the XML file.
  //##
  //## The function WriteXML() writes a complete XML node and its data (calls the functions BeginNode(), WriteXMLData() and EndNode()).
  //## Use function WriteXML() to write mitk classes to the XML file.
  //## e.g. <dataTreeNode CLASS_NAME="class mitk::DataTreeNode"> ...data... </dataTreeNode>
  //## For other elements e.g. the data of data types you can use the functions BeginNode(), EndNode() from the class BaseXMLWriter to write a XML node.
  //## @ingroup IO
  class XMLIO {
  
  public:

    virtual ~XMLIO() {};

    /// writes a XML node and its data (calls the functions BeginNode(), WriteXMLData() and EndNode()).
    bool WriteXML( XMLWriter& xmlWriter );

    virtual bool ReadXMLData( XMLReader& xmlReader );

    static const std::string CLASS_NAME;

  protected:

    virtual bool WriteXMLData( XMLWriter& xmlWriter );

    /// returns the name of the XML node
    virtual const std::string& GetXMLNodeName() const;

    static const std::string XML_NODE_NAME;
  };

}
#endif // XMLIO_H_HEADER_INCLUDED
