/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkContourModelWriter.h"
#include <iostream>
#include <fstream>
#include <locale>


/*
 * The xml file will look like:
 *
 *   <?xml version="1.0" encoding="utf-8"?>
 *   <contourModel>
 *      <head>
 *        <geometryInfo>
 *        </geometryInfo>
 *      </head>
 *      <data>
 *        <timestep n="0">
 *          <controlPoints>
 *            <point>
 *              <x></x>
 *              <y></y>
 *              <z></z>
 *            </point>
 *          </controlPoint>
 *        </timestep>
 *      </data>
 *    </contourModel>
 */


//
// Initialization of the xml tags.
//

const char* mitk::ContourModelWriter::XML_CONTOURMODEL = "contourModel" ;

const char* mitk::ContourModelWriter::XML_HEAD = "head" ;

const char* mitk::ContourModelWriter::XML_GEOMETRY_INFO = "geometryInfo" ;

const char* mitk::ContourModelWriter::XML_DATA = "data";

const char* mitk::ContourModelWriter::XML_TIME_STEP = "timestep";

const char* mitk::ContourModelWriter::XML_CONTROL_POINTS = "controlPoints" ;

const char* mitk::ContourModelWriter::XML_POINT = "point" ;

const char* mitk::ContourModelWriter::XML_X = "x" ;

const char* mitk::ContourModelWriter::XML_Y = "y" ;

const char* mitk::ContourModelWriter::XML_Z = "z" ;




mitk::ContourModelWriter::ContourModelWriter()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
    this->SetNumberOfRequiredInputs( 1 );
    this->SetNumberOfOutputs( 1 );
    this->SetNthOutput( 0, mitk::ContourModel::New().GetPointer() );
    m_Indent = 2;
    m_IndentDepth = 0;
    m_Success = false;
}




mitk::ContourModelWriter::~ContourModelWriter()
{}




void mitk::ContourModelWriter::GenerateData()
{
    m_Success = false;
    m_IndentDepth = 0;

    //
    // Opening the file to write to
    //
    if ( m_FileName == "" )
    {
        itkWarningMacro( << "Sorry, filename has not been set!" );
        return ;
    }
    std::ofstream out( m_FileName.c_str() );
    if ( !out.good() )
    {
      itkExceptionMacro(<< "File " << m_FileName << " could not be opened!");
      itkWarningMacro( << "Sorry, file " << m_FileName << " could not be opened!" );
      out.close();
        return ;
    }

    std::locale previousLocale(out.getloc());
    std::locale I("C");
    out.imbue(I);


/*+++++++++++ Here the actual xml writing begins +++++++++*/

    /*++++ <?xml version="1.0" encoding="utf-8"?> ++++*/
    WriteXMLHeader( out );
    /*++++ <contourModel> ++++*/
    WriteStartElement( XML_CONTOURMODEL, out );

    /*++++ <head> ++++*/
    WriteStartElement( XML_HEAD, out);

    /*++++ <geometryInfo> ++++*/
    WriteStartElement( XML_GEOMETRY_INFO, out);


    //write the geometry informations to the stream
      InputType::Pointer contourModel = this->GetInput();
      assert( contourModel.IsNotNull() );
      WriteGeometryInformation( contourModel->GetTimeSlicedGeometry(), out);;


    /*++++ </geometryInfo> ++++*/
    WriteEndElement( XML_GEOMETRY_INFO, out);

    /*++++ </head> ++++*/
    WriteEndElement( XML_HEAD, out);

    //
    // for each input object write its xml representation to
    // the stream
    //
    for ( unsigned int i = 0 ; i < this->GetNumberOfInputs(); ++i )
    {
        InputType::Pointer contourModel = this->GetInput( i );
        assert( contourModel.IsNotNull() );
        WriteXML( contourModel.GetPointer(), out );
    }


    /*++++ </contourModel> ++++*/
    WriteEndElement( XML_CONTOURMODEL, out );


    out.imbue(previousLocale);

    if ( !out.good() ) // some error during output
    {
      out.close();
      throw std::ios_base::failure("Some error during point set writing.");
    }

    out.close();
    m_Success = true;
    m_MimeType = "application/MITK.ContourModel";
}




void mitk::ContourModelWriter::WriteXML( mitk::ContourModel* contourModel, std::ofstream& out )
{
  /*++++ <data> ++++*/
  WriteStartElement( XML_DATA, out);

  unsigned int timecount = contourModel->GetTimeSteps();

  for(unsigned int i=0; i< timecount; i++)
  {
    /*++++ <timestep> ++++*/
    std::vector<std::string> at;
    at.push_back("n");
    std::vector<std::string> val;
    val.push_back(ConvertToString(i));

    at.push_back("isClosed");
    val.push_back(ConvertToString(contourModel->IsClosed()));

    WriteStartElementWithAttribut( XML_TIME_STEP, at, val, out );

    /*++++ <controlPoints> ++++*/
    WriteStartElement(XML_CONTROL_POINTS, out);

    mitk::ContourModel::VertexIterator it = contourModel->IteratorBegin();
    mitk::ContourModel::VertexIterator end = contourModel->IteratorEnd();


    while(it != end)
    {
      mitk::ContourModel::VertexType* v = *it;

      /*++++ <point> ++++*/
      std::vector<std::string> attr;
      attr.push_back("IsControlPoint");
      std::vector<std::string> value;
      value.push_back(ConvertToString(v->IsControlPoint));
      WriteStartElementWithAttribut( XML_POINT, attr, value, out );


      /*++++ <x> ++++*/
      WriteStartElement( XML_X, out );
      WriteCharacterData( ConvertToString(v->Coordinates[0] ).c_str(), out );
      /*++++ </x> ++++*/
      WriteEndElement( XML_X, out, false );

      /*++++ <y> ++++*/
      WriteStartElement( XML_Y, out );
      WriteCharacterData( ConvertToString( v->Coordinates[1] ).c_str(), out );
      /*++++ </y> ++++*/
      WriteEndElement( XML_Y, out, false );

      /*++++ <z> ++++*/
      WriteStartElement( XML_Z, out );
      WriteCharacterData( ConvertToString( v->Coordinates[2] ).c_str(), out );
      /*++++ </z> ++++*/
      WriteEndElement( XML_Z, out, false );

      /*++++ </point> ++++*/
      WriteEndElement( XML_POINT, out );

      it++;
    }

    /*++++ </controlPoints> ++++*/
    WriteEndElement(XML_CONTROL_POINTS, out);

    /*++++ </timestep> ++++*/
    WriteEndElement( XML_TIME_STEP, out );

  }

  /*++++ </data> ++++*/
  WriteEndElement( XML_DATA, out );
}



void mitk::ContourModelWriter::WriteGeometryInformation( mitk::TimeSlicedGeometry* geometry, std::ofstream& out )
{
  WriteCharacterData("<!-- geometry information -->", out);
}



void mitk::ContourModelWriter::ResizeInputs( const unsigned int& num )
{
    unsigned int prevNum = this->GetNumberOfInputs();
    this->SetNumberOfInputs( num );
    for ( unsigned int i = prevNum; i < num; ++i )
    {
        this->SetNthInput( i, mitk::ContourModel::New().GetPointer() );
    }
}




void mitk::ContourModelWriter::SetInput( InputType* contourModel )
{
    this->ProcessObject::SetNthInput( 0, contourModel );
}




void mitk::ContourModelWriter::SetInput( const unsigned int& id, InputType* contourModel )
{
    if ( id >= this->GetNumberOfInputs() )
        this->ResizeInputs( id + 1 );
    this->ProcessObject::SetNthInput( id, contourModel );
}



mitk::ContourModel* mitk::ContourModelWriter::GetInput()
{
    if ( this->GetNumberOfInputs() < 1 )
    {
        return 0;
    }
    else
    {
        return dynamic_cast<InputType*> ( this->GetInput( 0 ) );
    }
}




mitk::ContourModel* mitk::ContourModelWriter::GetInput( const unsigned int& num )
{
    return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( num ) );
}





template < typename T>
std::string mitk::ContourModelWriter::ConvertToString( T value )
{
    std::ostringstream o;
    std::locale I("C");
    o.imbue(I);

    if ( o << value )
    {
        return o.str();
     }
    else
        return "conversion error";
}



void mitk::ContourModelWriter::WriteXMLHeader( std::ofstream &file )
{
    file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}




void mitk::ContourModelWriter::WriteStartElement( const char *const tag, std::ofstream &file )
{
    file << std::endl;
    WriteIndent( file );
    file << '<' << tag << '>';
    m_IndentDepth++;
}



void mitk::ContourModelWriter::WriteStartElementWithAttribut( const char *const tag, std::vector<std::string> attributes, std::vector<std::string> values, std::ofstream &file )
{
    file << std::endl;
    WriteIndent( file );
    file << '<' << tag;

    unsigned int attributesSize = attributes.size();
    unsigned int valuesSize = values.size();

    if( attributesSize == valuesSize){
      std::vector<std::string>::iterator attributesIt = attributes.begin();
      std::vector<std::string>::iterator end = attributes.end();

      std::vector<std::string>::iterator valuesIt = values.begin();

      while(attributesIt != end)
      {
        file << ' ';
        WriteCharacterData( *attributesIt, file);
        file << '=' << '"';
        WriteCharacterData( *valuesIt, file);
        file << '"';
        attributesIt++;
        valuesIt++;
      }
    }


    file << '>';
    m_IndentDepth++;
}




void mitk::ContourModelWriter::WriteEndElement( const char *const tag, std::ofstream &file, const bool& indent )
{
    m_IndentDepth--;
    if ( indent )
    {
        file << std::endl;
        WriteIndent( file );
    }
    file << '<' << '/' << tag << '>';
}




void mitk::ContourModelWriter::WriteCharacterData( const char *const data, std::ofstream &file )
{
    file << data;
}




void mitk::ContourModelWriter::WriteStartElement( std::string &tag, std::ofstream &file )
{
    WriteStartElement( tag.c_str(), file );
}




void mitk::ContourModelWriter::WriteEndElement( std::string &tag, std::ofstream &file, const bool& indent )
{
    WriteEndElement( tag.c_str(), file, indent );
}




void mitk::ContourModelWriter::WriteCharacterData( std::string &data, std::ofstream &file )
{
    WriteCharacterData( data.c_str(), file );
}




void mitk::ContourModelWriter::WriteIndent( std::ofstream& file )
{
    std::string spaces( m_IndentDepth * m_Indent, ' ' );
    file << spaces.c_str();
}



bool mitk::ContourModelWriter::GetSuccess() const
{
    return m_Success;
}



bool mitk::ContourModelWriter::CanWriteDataType( DataNode* input )
{
  if ( input )
  {
    mitk::BaseData* data = input->GetData();
    if ( data )
    {
       mitk::ContourModel::Pointer contourModel = dynamic_cast<mitk::ContourModel*>( data );
       if( contourModel.IsNotNull() )
       {
         //this writer has no "SetDefaultExtension()" - function
         m_Extension = ".cnt";
         return true;
       }
    }
  }
  return false;
}

void mitk::ContourModelWriter::SetInput( DataNode* input )
{
  if( input && CanWriteDataType( input ) )
    this->ProcessObject::SetNthInput( 0, dynamic_cast<mitk::ContourModel*>( input->GetData() ) );
}

std::string mitk::ContourModelWriter::GetWritenMIMEType()
{
  return m_MimeType;
}

std::vector<std::string> mitk::ContourModelWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".cnt");
  return possibleFileExtensions;
}

std::string mitk::ContourModelWriter::GetFileExtension()
{
  return m_Extension;
}
