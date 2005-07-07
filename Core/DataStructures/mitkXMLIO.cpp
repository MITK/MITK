#include "mitkXMLIO.h"
#include <mitkXMLWriter.h>

namespace mitk {

/**
 *
 */
bool XMLIO::WriteXML( XMLWriter& xmlWriter ) 
{
	xmlWriter.BeginNode("mitkXMLIO");
	xmlWriter.WriteProperty( "className", typeid( *this ).name() );
	xmlWriter.EndNode();
	return true;				
}

/**
 *
 */
bool XMLIO::ReadXML( XMLReader& xmlReader )
{
	return false;
}

} // namespace mitk

