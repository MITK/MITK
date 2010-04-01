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


#include "mitkPointSetWriter.h"
#include <iostream>
#include <fstream>
#include <locale>


//
// Initialization of the xml tags.
//

const char* mitk::PointSetWriter::XML_POINT_SET_FILE = "point_set_file" ;

const char* mitk::PointSetWriter::XML_FILE_VERSION = "file_version" ;

const char* mitk::PointSetWriter::XML_POINT_SET = "point_set" ;

const char* mitk::PointSetWriter::XML_TIME_SERIES = "time_series";

const char* mitk::PointSetWriter::XML_TIME_SERIES_ID = "time_series_id";

const char* mitk::PointSetWriter::XML_POINT = "point" ;

const char* mitk::PointSetWriter::XML_ID = "id" ;

const char* mitk::PointSetWriter::XML_SPEC = "specification" ;

const char* mitk::PointSetWriter::XML_X = "x" ;

const char* mitk::PointSetWriter::XML_Y = "y" ;

const char* mitk::PointSetWriter::XML_Z = "z" ;

const char* mitk::PointSetWriter::VERSION_STRING = "0.1" ;




mitk::PointSetWriter::PointSetWriter()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
    this->SetNumberOfRequiredInputs( 1 );
    this->SetNumberOfOutputs( 1 );
    this->SetNthOutput( 0, mitk::PointSet::New().GetPointer() );
    m_Indent = 2;
    m_IndentDepth = 0;
    m_Success = false;
}




mitk::PointSetWriter::~PointSetWriter()
{}




void mitk::PointSetWriter::GenerateData()
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

    //
    // Here the actual xml writing begins
    //
    WriteXMLHeader( out );
    WriteStartElement( XML_POINT_SET_FILE, out );
    WriteStartElement( XML_FILE_VERSION, out );
    WriteCharacterData( VERSION_STRING, out );
    WriteEndElement( XML_FILE_VERSION, out, false );

    //
    // for each input object write its xml representation to
    // the stream
    //
    for ( unsigned int i = 0 ; i < this->GetNumberOfInputs(); ++i )
    {
        InputType::Pointer pointSet = this->GetInput( i );
        assert( pointSet.IsNotNull() );
        WriteXML( pointSet.GetPointer(), out );
    }

   WriteEndElement( XML_POINT_SET_FILE, out );
   out.imbue(previousLocale);
    if ( !out.good() ) // some error during output
    {
      out.close();
      throw std::ios_base::failure("Some error during point set writing.");
    }
 
    out.close();
    m_Success = true;
    m_MimeType = "application/MITK.PointSet";
}




void mitk::PointSetWriter::WriteXML( mitk::PointSet* pointSet, std::ofstream& out )
{
    WriteStartElement( XML_POINT_SET, out );
    mitk::PointSet::PointsContainer* pointsContainer = pointSet->GetPointSet()->GetPoints();
    mitk::PointSet::PointsContainer::Iterator it;
    
    unsigned int timecount = pointSet->GetTimeSteps();

    for(unsigned int i=0; i< timecount; i++)
    {
      WriteStartElement( XML_TIME_SERIES, out );
      
      WriteStartElement( XML_TIME_SERIES_ID, out );
      WriteCharacterData( ConvertToString( i ).c_str() , out );
      WriteEndElement( XML_TIME_SERIES_ID, out, false );

      for ( it = pointsContainer->Begin(); it != pointsContainer->End(); ++it )
      {
          WriteStartElement( XML_POINT, out );

          WriteStartElement( XML_ID, out );
          WriteCharacterData( ConvertToString( it->Index() ).c_str() , out );
          WriteEndElement( XML_ID, out, false );

          mitk::PointSet::PointType point = it->Value();

          WriteStartElement( XML_SPEC, out );
          WriteCharacterData( ConvertToString( pointSet->GetSpecificationTypeInfo(it->Index(), i) ).c_str() , out );
          WriteEndElement( XML_SPEC, out, false );

          WriteStartElement( XML_X, out );
          WriteCharacterData( ConvertToString( point[ 0 ] ).c_str(), out );
          WriteEndElement( XML_X, out, false );

          WriteStartElement( XML_Y, out );
          WriteCharacterData( ConvertToString( point[ 1 ] ).c_str(), out );
          WriteEndElement( XML_Y, out, false );

          WriteStartElement( XML_Z, out );
          WriteCharacterData( ConvertToString( point[ 2 ] ).c_str(), out );
          WriteEndElement( XML_Z, out, false );

          WriteEndElement( XML_POINT, out );
      }
    WriteEndElement( XML_TIME_SERIES, out, false );
    }

    WriteEndElement( XML_POINT_SET, out );
}





void mitk::PointSetWriter::ResizeInputs( const unsigned int& num )
{
    unsigned int prevNum = this->GetNumberOfInputs();
    this->SetNumberOfInputs( num );
    for ( unsigned int i = prevNum; i < num; ++i )
    {
        this->SetNthInput( i, mitk::PointSet::New().GetPointer() );
    }
}




void mitk::PointSetWriter::SetInput( InputType* pointSet )
{
    this->ProcessObject::SetNthInput( 0, pointSet );
}




void mitk::PointSetWriter::SetInput( const unsigned int& id, InputType* pointSet )
{
    if ( id >= this->GetNumberOfInputs() )
        this->ResizeInputs( id + 1 );
    this->ProcessObject::SetNthInput( id, pointSet );
}



mitk::PointSet* mitk::PointSetWriter::GetInput()
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




mitk::PointSet* mitk::PointSetWriter::GetInput( const unsigned int& num )
{
    return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( num ) );
}





template < typename T>
std::string mitk::PointSetWriter::ConvertToString( T value )
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



void mitk::PointSetWriter::WriteXMLHeader( std::ofstream &file )
{
    file << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>";
}




void mitk::PointSetWriter::WriteStartElement( const char *const tag, std::ofstream &file )
{
    file << std::endl;
    WriteIndent( file );
    file << '<' << tag << '>';
    m_IndentDepth++;
}




void mitk::PointSetWriter::WriteEndElement( const char *const tag, std::ofstream &file, const bool& indent )
{
    m_IndentDepth--;
    if ( indent )
    {
        file << std::endl;
        WriteIndent( file );
    }
    file << '<' << '/' << tag << '>';
}




void mitk::PointSetWriter::WriteCharacterData( const char *const data, std::ofstream &file )
{
    file << data;
}




void mitk::PointSetWriter::WriteStartElement( std::string &tag, std::ofstream &file )
{
    WriteStartElement( tag.c_str(), file );
}




void mitk::PointSetWriter::WriteEndElement( std::string &tag, std::ofstream &file, const bool& indent )
{
    WriteEndElement( tag.c_str(), file, indent );
}




void mitk::PointSetWriter::WriteCharacterData( std::string &data, std::ofstream &file )
{
    WriteCharacterData( data.c_str(), file );
}




void mitk::PointSetWriter::WriteIndent( std::ofstream& file )
{
    std::string spaces( m_IndentDepth * m_Indent, ' ' );
    file << spaces.c_str();
}



bool mitk::PointSetWriter::GetSuccess() const
{
    return m_Success;  
}



bool mitk::PointSetWriter::CanWriteDataType( DataNode* input )
{
  if ( input )
  {
    mitk::BaseData* data = input->GetData();
    if ( data )
    {
       mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>( data );
       if( pointSet.IsNotNull() )
       {
         //this writer has no "SetDefaultExtension()" - function 
         m_Extension = ".mps";
         return true;
       }
    }
  }
  return false;
}

void mitk::PointSetWriter::SetInput( DataNode* input )
{
  if( input && CanWriteDataType( input ) )
    this->ProcessObject::SetNthInput( 0, dynamic_cast<mitk::PointSet*>( input->GetData() ) );
}

std::string mitk::PointSetWriter::GetWritenMIMEType()
{
  return m_MimeType;
}

std::vector<std::string> mitk::PointSetWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".mps");
  return possibleFileExtensions;
}

std::string mitk::PointSetWriter::GetFileExtension()
{
  return m_Extension;
}
