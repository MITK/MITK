/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkCommon.h"

namespace mitk {

  class XMLWriter;
  class XMLReader;

  //##Documentation
  //## @brief XMLIO class manages the writing and reading of data to a XML file.
  //##
  //## The function WriteXML() writes a complete XML node and its data (calls the functions BeginNode(), WriteProperty(), WriteXMLData() and EndNode() of the XMLWriter class).
  //## Normally the function WriteXML() is used to write mitk classes to the XML file
  //## e.g. <dataTreeNode CLASS_NAME="class mitk::DataTreeNode"> ...data... </dataTreeNode>
  //## To write the data of a XML node (e.g. child nodes, attributes) call the function WriteXMLData().
  //## For elements like the data of data types you can use the functions BeginNode(), EndNode() from the BaseXMLWriter class to write a XML node.
  //## To read data from the XML file call the function ReadXMLData().
  //##
  //## New classes should be derived from this class for writing/ reading and should be defined in the
  //## MapClassIDToClassName class and in the ObjectFactory class.
  //## What should be implemented in the functions WriteXMLData() and ReadXMLData() of new classes?
  //## Just have a look to the documentation of the module IO classes.
  //## @ingroup IO
  class MITK_CORE_EXPORT XMLIO {
  
  public:

    virtual ~XMLIO() {};
    
    virtual const char* GetNameOfClass() const = 0;

    /// writes a XML node and its data (calls the functions BeginNode(), WriteXMLData() and EndNode()).
    bool WriteXML( XMLWriter& xmlWriter );

    virtual bool ReadXMLData( XMLReader& xmlReader );

    static const std::string CLASS_NAME;

  protected:

    /// writes the data of a XML node (child nodes, attributes)
    virtual bool WriteXMLData( XMLWriter& xmlWriter );

    /// returns the name of the XML node
    virtual const std::string& GetXMLNodeName() const;

    static const std::string XML_NODE_NAME;
  };

}
#endif // XMLIO_H_HEADER_INCLUDED
