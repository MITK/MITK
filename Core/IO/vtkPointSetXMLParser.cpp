#include "vtkPointSetXMLParser.h"
#include "mitkPointSetWriter.h"
#include "mitkOperation.h"
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"

void mitk::vtkPointSetXMLParser::StartElement ( const char *name, const char **atts )
{
    std::string currentElement = name;
    if ( currentElement == mitk::PointSetWriter::XML_POINT_SET )
    {
        mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
        m_PointSetList.push_back( pointSet );
        m_CurrentPointSet = pointSet;
    }
    else if ( currentElement == mitk::PointSetWriter::XML_POINT )
    {
        m_CurrentPoint[ 0 ] = 0.0f;
        m_CurrentPoint[ 1 ] = 0.0f;
        m_CurrentPoint[ 2 ] = 0.0f;
    }
    m_ParseStack.push( currentElement );
}

void mitk::vtkPointSetXMLParser::EndElement ( const char *name )
{
    std::string currentElement = name;
    if ( m_ParseStack.top() != currentElement )
    {
        std::cerr << "Top of parse stack ( " << m_ParseStack.top() << " ) is != currentEndElement ( " << currentElement << " )!" << std::endl;
    }
    m_ParseStack.pop();

    if ( currentElement == mitk::PointSetWriter::XML_POINT )
    {
        mitk::PointOperation pop( mitk::OpINSERT, m_CurrentPoint, m_CurrentPointId );
        assert( m_CurrentPointSet.IsNotNull() );
        m_CurrentPointSet->ExecuteOperation( &pop );
    }
}

void mitk::vtkPointSetXMLParser::CharacterDataHandler ( const char *inData, int inLength )
{
    std::string currentElement = m_ParseStack.top();
    if ( currentElement == mitk::PointSetWriter::XML_ID )
    {
        m_CurrentPointId = ParsePointIdentifier( inData, inLength );
    }
    if ( currentElement == mitk::PointSetWriter::XML_X )
    {
        m_CurrentPoint[ 0 ] = ParseScalarType( inData, inLength );
    }
    if ( currentElement == mitk::PointSetWriter::XML_Y )
    {
        m_CurrentPoint[ 1 ] = ParseScalarType( inData, inLength );
    }
    if ( currentElement == mitk::PointSetWriter::XML_Z )
    {
        m_CurrentPoint[ 2 ] = ParseScalarType( inData, inLength );
    }
}


mitk::ScalarType mitk::vtkPointSetXMLParser::ParseScalarType( const char *inData, int inLength )
{
    std::string data( inData, inLength );
    return ( mitk::ScalarType ) atof( data.c_str() );
}


mitk::vtkPointSetXMLParser::PointIdentifier mitk::vtkPointSetXMLParser::ParsePointIdentifier( const char *inData, int inLength )
{
    std::string data( inData, inLength );
    return ( mitk::vtkPointSetXMLParser::PointIdentifier ) atol( data.c_str() );
}

mitk::vtkPointSetXMLParser::PointSetList mitk::vtkPointSetXMLParser::GetParsedPointSets()
{
    return m_PointSetList;
}
