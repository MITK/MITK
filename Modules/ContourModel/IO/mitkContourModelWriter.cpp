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
#include "mitkIOMimeTypes.h"
#include "mitkTimeGeometry.h"
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
  : AbstractFileWriter(ContourModel::GetStaticNameOfClass())
  , m_IndentDepth(0)
  , m_Indent(2)
{
  std::string category = "Contour File";
  mitk::CustomMimeType customMimeType;
  customMimeType.SetCategory(category);
  customMimeType.AddExtension("cnt");

  this->SetDescription(category);
  this->SetMimeType(customMimeType);

  RegisterService();
}

mitk::ContourModelWriter::ContourModelWriter(const mitk::ContourModelWriter& other)
  : AbstractFileWriter(other)
  , m_IndentDepth(other.m_IndentDepth)
  , m_Indent(other.m_Indent)
{
}

mitk::ContourModelWriter::~ContourModelWriter()
{}

void mitk::ContourModelWriter::Write()
{
  std::ostream* out;
  std::ofstream outStream;

  if( this->GetOutputStream() )
  {
    out = this->GetOutputStream();
  }
  else
  {
    outStream.open( this->GetOutputLocation().c_str() );
    out = &outStream;
  }

  if ( !out->good() )
  {
    mitkThrow() << "Stream not good.";
  }

  std::locale previousLocale(out->getloc());
  std::locale I("C");
  out->imbue(I);

  /*+++++++++++ Here the actual xml writing begins +++++++++*/

    /*++++ <?xml version="1.0" encoding="utf-8"?> ++++*/
    WriteXMLHeader( *out );


    //
    // for each input object write its xml representation to
    // the stream
    //
    mitk::ContourModel::ConstPointer contourModel = dynamic_cast<const mitk::ContourModel*>(this->GetInput());
    assert( contourModel.IsNotNull() );
    WriteXML( contourModel.GetPointer(), *out );

    out->imbue(previousLocale);

    if ( !out->good() ) // some error during output
    {
      throw std::ios_base::failure("Some error during contour writing.");
    }

}

mitk::ContourModelWriter*mitk::ContourModelWriter::Clone() const
{
  return new ContourModelWriter(*this);
}


void mitk::ContourModelWriter::WriteXML( const mitk::ContourModel* contourModel, std::ostream& out )
{
   /*++++ <contourModel> ++++*/
    WriteStartElement( XML_CONTOURMODEL, out );

    /*++++ <head> ++++*/
    WriteStartElement( XML_HEAD, out);

    /*++++ <geometryInfo> ++++*/
    WriteStartElement( XML_GEOMETRY_INFO, out);

    WriteGeometryInformation( contourModel->GetTimeGeometry(), out);

    /*++++ </geometryInfo> ++++*/
    WriteEndElement( XML_GEOMETRY_INFO, out);

    /*++++ </head> ++++*/
    WriteEndElement( XML_HEAD, out);

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

  /*++++ </contourModel> ++++*/
    WriteEndElement( XML_CONTOURMODEL, out );
}

void mitk::ContourModelWriter::WriteGeometryInformation(const mitk::TimeGeometry* /*geometry*/, std::ostream& out )
{
  WriteCharacterData("<!-- geometry information -->", out);
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

void mitk::ContourModelWriter::WriteXMLHeader( std::ostream &file )
{
    file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}

void mitk::ContourModelWriter::WriteStartElement( const char *const tag, std::ostream &file )
{
    file << std::endl;
    WriteIndent( file );
    file << '<' << tag << '>';
    m_IndentDepth++;
}

void mitk::ContourModelWriter::WriteStartElementWithAttribut( const char *const tag, std::vector<std::string> attributes, std::vector<std::string> values, std::ostream &file )
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

void mitk::ContourModelWriter::WriteEndElement( const char *const tag, std::ostream &file, const bool& indent )
{
    m_IndentDepth--;
    if ( indent )
    {
        file << std::endl;
        WriteIndent( file );
    }
    file << '<' << '/' << tag << '>';
}

void mitk::ContourModelWriter::WriteCharacterData( const char *const data, std::ostream &file )
{
    file << data;
}

void mitk::ContourModelWriter::WriteStartElement( std::string &tag, std::ostream &file )
{
    WriteStartElement( tag.c_str(), file );
}

void mitk::ContourModelWriter::WriteEndElement( std::string &tag, std::ostream &file, const bool& indent )
{
    WriteEndElement( tag.c_str(), file, indent );
}

void mitk::ContourModelWriter::WriteCharacterData( std::string &data, std::ostream &file )
{
    WriteCharacterData( data.c_str(), file );
}

void mitk::ContourModelWriter::WriteIndent( std::ostream& file )
{
    std::string spaces( m_IndentDepth * m_Indent, ' ' );
    file << spaces.c_str();
}
