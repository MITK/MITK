#include "vtkPointSetXMLParser.h"
#include "mitkPointSetWriter.h"
#include "mitkOperation.h"
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"




void mitk::vtkPointSetXMLParser::StartElement ( const char *name, const char **atts )
{
    std::string currentElement = name;
    //
    // when a new point set begins in the file, create a new
    // mitk::point set and store it in m_PointSetList
    //
    if ( currentElement == mitk::PointSetWriter::XML_POINT_SET )
    {
        mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
        m_PointSetList.push_back( pointSet );
        m_CurrentPointSet = pointSet;
    }
    //
    // when a new point begins, initialize it to zero.
    //
    else if ( currentElement == mitk::PointSetWriter::XML_POINT )
    {
        m_CurrentPoint[ 0 ] = 0.0f;
        m_CurrentPoint[ 1 ] = 0.0f;
        m_CurrentPoint[ 2 ] = 0.0f;
    }
    
    //
    // the current element is pushed on to the stack
    // to be able to detect some errors in the xml file
    //
    m_ParseStack.push( currentElement );
}




void mitk::vtkPointSetXMLParser::EndElement ( const char *name )
{
    std::string currentElement = name;
    
    //
    // make sure, that the current end element matches with the 
    // last start tag
    //
    if ( m_ParseStack.top() != currentElement )
    {
        std::cerr << "Top of parse stack ( " << m_ParseStack.top() << " ) is != currentEndElement ( " << currentElement << " )!" << std::endl;
    }
    m_ParseStack.pop();

    //
    // if we have finished parsing a point, insert it to the current
    // point set.
    //
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
    else if ( currentElement == mitk::PointSetWriter::XML_X )
    {
        m_CurrentPoint[ 0 ] = ParseScalarType( inData, inLength );
    }
    else if ( currentElement == mitk::PointSetWriter::XML_Y )
    {
        m_CurrentPoint[ 1 ] = ParseScalarType( inData, inLength );
    }
    else if ( currentElement == mitk::PointSetWriter::XML_Z )
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



