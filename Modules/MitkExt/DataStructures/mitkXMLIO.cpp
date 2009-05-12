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

#include "mitkXMLIO.h"
#include <mitkXMLWriter.h>
#include <iostream>

namespace mitk {

const std::string XMLIO::XML_NODE_NAME = "nameless";
const std::string XMLIO::CLASS_NAME = "CLASS_NAME";


bool XMLIO::WriteXML( XMLWriter& xmlWriter ) 
{
  const std::string& nodeName = GetXMLNodeName();
  std::cout << nodeName.c_str() << std::endl; // test
  xmlWriter.BeginNode( nodeName );
  xmlWriter.WriteProperty( CLASS_NAME, this->GetNameOfClass() );

  bool result = WriteXMLData( xmlWriter );

  xmlWriter.EndNode();
  return result;        
}


bool XMLIO::WriteXMLData( XMLWriter& /*xmlWriter*/ )
{
  return true;
}


bool XMLIO::ReadXMLData( XMLReader& /*xmlReader*/ )
{
 return false;
}


const std::string& XMLIO::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}

} // namespace mitk
