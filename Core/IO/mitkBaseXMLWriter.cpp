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

#include "mitkBaseXMLWriter.h"

#include <stdio.h>
#include <stdlib.h>

namespace mitk {

const std::string BaseXMLWriter::FILE_VERSION_KEY = "FILE_VERSION";
const std::string BaseXMLWriter::VERSION = "0.1";

BaseXMLWriter::BaseXMLWriter( const char* filename, int space )
:m_Out(NULL), m_Increase(0), m_Space(space), m_NodeCount(0) , m_File(true), m_FirstNode(true), m_XMLPathAndFilename(filename)

{   
  m_Out = new std::ofstream( filename );
}


/*
BaseXMLWriter::BaseXMLWriter( std::ostringstream& out, int space )
:m_Out(&out), m_Increase(0), m_Space(space), m_NodeCount(0), m_File(false), m_FirstNode(true) 
{
  // nothing to do
}
*/

BaseXMLWriter::~BaseXMLWriter() 
{ 
  if ( !m_Out )
    return;

  m_Out->flush();

  if ( m_File ) 
  {
    std::ofstream* file = static_cast<std::ofstream*>(m_Out);
    file->close();
    delete file;
  }
}

/**
  begins a new XML node

  \ param name specifies the name of the XML node
*/
void BaseXMLWriter::BeginNode( const std::string& name ) 
{   
  if ( m_FirstNode ) 
  {
    *m_Out << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
    m_FirstNode = false;
    m_Stack.push( new StreamNode( name ) );
    m_Increase++;
    m_NodeCount++;
    WriteProperty(FILE_VERSION_KEY, VERSION);
  }
  else{
    m_Stack.push( new StreamNode( name ) );
  
    m_Increase++;
    m_NodeCount++;
  }
}


void BaseXMLWriter::EndNode( )
{     
  m_Increase--;

  int steps = m_Space * m_Increase;
  StreamNode* current = m_Stack.top();
  m_Stack.pop();

  if ( m_Stack.size() )
  {
    std::ostream& stream = m_Stack.top()->GetChildrenStream();
    current->Write( stream, steps );      
  }
  else
  {
    current->Write( *m_Out, steps );
  }
  delete current;
}

/**
  writes a XML attribute that datatype is a const char*

  \ param key specifies the name of the attribute
  \ param value represents the data of the attribute
*/
void BaseXMLWriter::WriteProperty( const std::string& key, const std::string& value ) const 
{
  std::ostream& stream = m_Stack.top()->GetPropertyStream();
  stream << ConvertString( key.c_str() );
  stream << "=\"" << ConvertString( value.c_str() ) << "\" ";
}

void BaseXMLWriter::WriteComment( const std::string& text ) 
{
  std::ostream& stream = m_Stack.top()->GetComment();

  if ( m_FirstNode ) 
  {
    stream <<"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
    m_FirstNode = false;
  }

  stream << ConvertString( text.c_str() );
}
  

unsigned int BaseXMLWriter::GetCurrentDeph() const
{
  return static_cast<unsigned int>(m_Stack.size());
}


int BaseXMLWriter::GetNodeCount() const
{
  return m_NodeCount;
}


int BaseXMLWriter::GetSpace() const
{
  return m_Space;
}


void BaseXMLWriter::SetSpace( int space )
{
  m_Space = space;
}


std::string BaseXMLWriter::ConvertString( const std::string& s ) const
{
  std::string workString(s);

  std::string::size_type index = workString.find('<');
  while ( index != std::string::npos )
  {
    workString[index] = '{';
    index = workString.find('<', index);
  }

  index = workString.find('>');
  while ( index != std::string::npos )
  {
    workString[index] = '}';
    index = workString.find('>', index);
  }

  return workString;
}


void BaseXMLWriter::StreamNode::Write( std::ostream& out, int steps )
{
  std::string comment = m_Comment.str();

  if ( comment.size() )
  {
    for(int i=0; i < steps; i++) out << ' '; 
    out << "<!-- " << comment << " -->\n";
  }

  for(int i=0; i < steps; i++) out << ' '; 

  out << '<' << m_Name;

  std::string properties = m_PropertyStream.str();

  if ( properties.size() )
    out << ' ' << properties;

  std::string children = m_ChildrenStream.str();

  if ( children.size() )
  {
    out << ">\n";
    out << children;
    for(int i=0; i < steps; i++) out << ' '; 
    out << "</" << m_Name << ">\n";
  }
  else
  {
    out << "/>\n";
  }
}


const std::string BaseXMLWriter::GetXMLPathAndFileName()
{
  return m_XMLPathAndFilename;
}

} // namespace mitk

