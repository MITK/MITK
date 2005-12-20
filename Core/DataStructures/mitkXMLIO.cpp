#include "mitkXMLIO.h"
#include <mitkXMLWriter.h>

namespace mitk {

const std::string XMLIO::XML_NODE_NAME = "nameless";
const std::string XMLIO::CLASS_NAME = "CLASS_NAME";


bool XMLIO::WriteXML( XMLWriter& xmlWriter ) 
{
  const std::string& nodeName = GetXMLNodeName();
  std::cout << nodeName.c_str() << std::endl; // test
	xmlWriter.BeginNode( nodeName );
	xmlWriter.WriteProperty( CLASS_NAME, typeid( *this ).name() );

  bool result = WriteXMLData( xmlWriter );

	xmlWriter.EndNode();
	return result;				
}


bool XMLIO::WriteXMLData( XMLWriter& xmlWriter )
{
  return true;
}


bool XMLIO::ReadXMLData( XMLReader& xmlReader )
{
 return false;
}


const std::string& XMLIO::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}

} // namespace mitk
