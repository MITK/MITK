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

#include "vtkPointSetXMLParser.h"
#include "vtkObjectFactory.h"
#include "mitkPointSetWriter.h"
#include "mitkOperation.h"
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"

namespace mitk
{
vtkStandardNewMacro(vtkPointSetXMLParser);
}

mitk::vtkPointSetXMLParser::vtkPointSetXMLParser()
{
}

mitk::vtkPointSetXMLParser::~vtkPointSetXMLParser()
{
}

int mitk::vtkPointSetXMLParser::InitializeParser()
{
   vtkXMLParser::InitializeParser();
   std::istream* stream = this -> GetStream();
   if (!stream)
   {
      vtkErrorMacro("no stream available in XML file reader");
       this->ParseError = 1;
       return 0;
   }
   m_PreviousLocale = stream->getloc();
   std::locale I("C");
   stream->imbue(I);
   return 1;
}

int mitk::vtkPointSetXMLParser::CleanupParser()
{
  std::istream* stream = this -> GetStream();
   if (!stream)
   {
      vtkErrorMacro("no stream available in XML file reader");
        this->ParseError = 1;
        return 0;
   }
   stream->imbue( m_PreviousLocale );
   vtkXMLParser::CleanupParser();
   return 1;
}



void mitk::vtkPointSetXMLParser::StartElement ( const char *name, const char ** /*atts */)
{
    std::string currentElement = name;
    //
    // when a new point set begins in the file, create a new
    // mitk::point set and store it in m_PointSetList
    //
    if ( currentElement == mitk::PointSetWriter::XML_POINT_SET )
    {
        m_CurrentPointSet = PointSetType::New();
    }
    //
    // when a new point begins, initialize it to zero.
    //
    else if ( currentElement == mitk::PointSetWriter::XML_POINT )
    {
        m_CurrentPoint[ 0 ] = 0.0f;
        m_CurrentPoint[ 1 ] = 0.0f;
        m_CurrentPoint[ 2 ] = 0.0f;
        m_CurId.clear();
        m_CurXString.clear();
        m_CurYString.clear();
        m_CurZString.clear();
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
        MITK_ERROR << "Top of parse stack ( " << m_ParseStack.top() << " ) is != currentEndElement ( " << currentElement << " )!" << std::endl;
    }
    m_ParseStack.pop();


    //
    // After a complete point set has been parsed, its
    // output information is updated and it is inserted into the list
    // of parsed point sets.
    //
    if (currentElement == mitk::PointSetWriter::XML_POINT_SET)
    {
        m_CurrentPointSet->UpdateOutputInformation();
        m_PointSetList.push_back( m_CurrentPointSet );
    }
    //
    // if we have finished parsing a point, insert it to the current
    // point set.
    //
    else if ( currentElement == mitk::PointSetWriter::XML_POINT )
    {
        m_CurrentPointId = ParsePointIdentifier( m_CurId );
        m_CurrentPoint[ 0 ] = ParseScalarType( m_CurXString );
        m_CurrentPoint[ 1 ] = ParseScalarType( m_CurYString );
        m_CurrentPoint[ 2 ] = ParseScalarType( m_CurZString );

        mitk::PointOperation popInsert( mitk::OpINSERT, m_CurrentPoint, m_CurrentPointId );
        mitk::PointOperation popDeactivate( mitk::OpDESELECTPOINT, m_CurrentPoint, m_CurrentPointId );
        assert( m_CurrentPointSet.IsNotNull() );
        m_CurrentPointSet->ExecuteOperation( &popInsert );
        m_CurrentPointSet->ExecuteOperation( &popDeactivate );
    }
}




void mitk::vtkPointSetXMLParser::CharacterDataHandler ( const char *inData, int inLength )
{
    std::string currentElement = m_ParseStack.top();
    if ( currentElement == mitk::PointSetWriter::XML_ID )
    {
        m_CurId.append( inData, inLength );        
    }
    else if ( currentElement == mitk::PointSetWriter::XML_X )
    { 
        m_CurXString.append(inData, inLength);
    }
    else if ( currentElement == mitk::PointSetWriter::XML_Y )
    {
        m_CurYString.append(inData, inLength);
    }
    else if ( currentElement == mitk::PointSetWriter::XML_Z )
    {
        m_CurZString.append(inData, inLength);
    }
}




mitk::ScalarType mitk::vtkPointSetXMLParser::ParseScalarType( const std::string &data )
{
    std::istringstream stm;
    stm.str(data);
    ScalarType number;
    stm >>number;
     return number;
}




mitk::vtkPointSetXMLParser::PointIdentifier mitk::vtkPointSetXMLParser::ParsePointIdentifier( const std::string &data )
{
   std::istringstream stm;
   stm.str(data);
   PointIdentifier pointID;
   stm >>pointID;
    return pointID;
}




mitk::vtkPointSetXMLParser::PointSetList mitk::vtkPointSetXMLParser::GetParsedPointSets()
{
    return m_PointSetList;
}



