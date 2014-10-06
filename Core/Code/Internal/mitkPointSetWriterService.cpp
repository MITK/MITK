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

#include "mitkPointSetWriterService.h"
#include "mitkIOMimeTypes.h"

#include <iostream>
#include <fstream>
#include <locale>

//
// Initialization of the xml tags.
//
const std::string mitk::PointSetWriterService::XML_POINT_SET_FILE = "point_set_file" ;
const std::string mitk::PointSetWriterService::XML_FILE_VERSION = "file_version" ;
const std::string mitk::PointSetWriterService::XML_POINT_SET = "point_set" ;
const std::string mitk::PointSetWriterService::XML_TIME_SERIES = "time_series";
const std::string mitk::PointSetWriterService::XML_TIME_SERIES_ID = "time_series_id";
const std::string mitk::PointSetWriterService::XML_POINT = "point" ;
const std::string mitk::PointSetWriterService::XML_ID = "id" ;
const std::string mitk::PointSetWriterService::XML_SPEC = "specification" ;
const std::string mitk::PointSetWriterService::XML_X = "x" ;
const std::string mitk::PointSetWriterService::XML_Y = "y" ;
const std::string mitk::PointSetWriterService::XML_Z = "z" ;
const std::string mitk::PointSetWriterService::VERSION_STRING = "0.1" ;


mitk::PointSetWriterService::PointSetWriterService()
  : AbstractFileWriter(PointSet::GetStaticNameOfClass(),
                       CustomMimeType(IOMimeTypes::POINTSET_MIMETYPE_NAME()),
                       "MITK Point Set Writer")
  , m_IndentDepth(0)
  , m_Indent(2)
{
  RegisterService();
}

mitk::PointSetWriterService::PointSetWriterService(const mitk::PointSetWriterService& other)
  : AbstractFileWriter(other)
  , m_IndentDepth(other.m_IndentDepth)
  , m_Indent(other.m_Indent)
{
}

mitk::PointSetWriterService::~PointSetWriterService()
{}

void mitk::PointSetWriterService::Write()
{
  OutputStream out(this);

  if ( !out.good() )
  {
    mitkThrow() << "Stream not good.";
  }

  std::locale previousLocale(out.getloc());
  std::locale I("C");
  out.imbue(I);

  //
  // Here the actual xml writing begins
  //
  WriteXMLHeader( out );
  WriteStartElement( XML_POINT_SET_FILE, out );
  WriteStartElement( XML_FILE_VERSION, out );
  out << VERSION_STRING;
  WriteEndElement( XML_FILE_VERSION, out, false );

  WriteXML( static_cast<const PointSet*>(this->GetInput()), out );

  WriteEndElement( XML_POINT_SET_FILE, out );

  out.imbue(previousLocale);
  if ( !out.good() ) // some error during output
  {
    mitkThrow() << "Some error during point set writing.";
  }
}

mitk::PointSetWriterService*mitk::PointSetWriterService::Clone() const
{
  return new PointSetWriterService(*this);
}

void mitk::PointSetWriterService::WriteXML( const mitk::PointSet* pointSet, std::ostream& out )
{
  WriteStartElement( XML_POINT_SET, out );
  unsigned int timecount = pointSet->GetTimeSteps();

  for(unsigned int i=0; i< timecount; i++)
  {
    WriteStartElement( XML_TIME_SERIES, out );

    WriteStartElement( XML_TIME_SERIES_ID, out );
    out << ConvertToString( i );
    WriteEndElement( XML_TIME_SERIES_ID, out, false );

    mitk::PointSet::PointsContainer* pointsContainer = pointSet->GetPointSet(i)->GetPoints();
    mitk::PointSet::PointsContainer::Iterator it;

    for ( it = pointsContainer->Begin(); it != pointsContainer->End(); ++it )
    {
      WriteStartElement( XML_POINT, out );

      WriteStartElement( XML_ID, out );
      out << ConvertToString( it->Index() );
      WriteEndElement( XML_ID, out, false );

      mitk::PointSet::PointType point = it->Value();

      WriteStartElement( XML_SPEC, out );
      out << ConvertToString( pointSet->GetSpecificationTypeInfo(it->Index(), i) );
      WriteEndElement( XML_SPEC, out, false );

      WriteStartElement( XML_X, out );
      out << ConvertToString( point[ 0 ] );
      WriteEndElement( XML_X, out, false );

      WriteStartElement( XML_Y, out );
      out << ConvertToString( point[ 1 ] );
      WriteEndElement( XML_Y, out, false );

      WriteStartElement( XML_Z, out );
      out << ConvertToString( point[ 2 ] );
      WriteEndElement( XML_Z, out, false );

      WriteEndElement( XML_POINT, out );
    }
    WriteEndElement( XML_TIME_SERIES, out );
  }

  WriteEndElement( XML_POINT_SET, out );
}

template < typename T>
std::string mitk::PointSetWriterService::ConvertToString( T value )
{
  std::ostringstream o;
  std::locale I("C");
  o.imbue(I);

  if ( o << value )
  {
    return o.str();
  }
  else
  {
    return "conversion error";
  }
}

void mitk::PointSetWriterService::WriteXMLHeader( std::ostream &file )
{
  file << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>";
}

void mitk::PointSetWriterService::WriteStartElement( const std::string& tag, std::ostream &file )
{
  file << std::endl;
  WriteIndent( file );
  file << '<' << tag << '>';
  m_IndentDepth++;
}

void mitk::PointSetWriterService::WriteEndElement( const std::string& tag, std::ostream &file, const bool& indent )
{
  m_IndentDepth--;
  if ( indent )
  {
    file << std::endl;
    WriteIndent( file );
  }
  file << '<' << '/' << tag << '>';
}

void mitk::PointSetWriterService::WriteIndent( std::ostream& file )
{
  std::string spaces( m_IndentDepth * m_Indent, ' ' );
  file << spaces;
}
