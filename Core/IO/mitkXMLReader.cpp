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

#include "mitkXMLReader.h"

#include <vtkXMLDataElement.h>
#include <mitkStatusBar.h>
#include <mitkCoreObjectFactory.h>
#include <mitkBaseRenderer.h>
#include <mitkGlobalInteraction.h>
#include <mitkEventMapper.h>
#include <mitkRenderWindow.h>
#include <mitkBaseXMLWriter.h>
#include <itksys/SystemTools.hxx>
#include <vtkObjectFactory.h>

#include <fstream>
#include <string>
#include <stack>

namespace mitk {

  const std::string XMLReader::DATA = "data";
  const std::string XMLReader::TREE_NODE = "treeNode";
  const std::string XMLReader::DATA_TREE_NODE = "dataTreeNode";
  const std::string XMLReader::ISTRUE = "1";
  const std::string XMLReader::ISFALSE = "0";
  const std::string XMLReader::PROPERTY = "property";
  const std::string XMLReader::DATA_TREE = "mitkDataTree";
  const std::string XMLReader::PROPERTY_KEY = "PROPERTY_KEY";
  const std::string XMLReader::PROPERTY_UID = "PROPERTY_UID";
  const std::string XMLReader::RENDERER = "renderer";
  const std::string XMLReader::RENDERER_NAME = "RENDERER_NAME";
  const std::string XMLReader::PROPERTY_LIST = "propertyList";
  const std::string XMLReader::FILENAME = "FILENAME";
  const std::string XMLReader::MIN = "MIN";
  const std::string XMLReader::MAX = "MAX";
  const std::string XMLReader::VALID = "VALID";
  
  std::string m_FileName;

  vtkCxxRevisionMacro( XMLReader, "$Revision$" );
  vtkStandardNewMacro( XMLReader );


  XMLReader::XMLReader()
    :vtkXMLParser(), m_Root(NULL), m_CurrentNode(NULL), m_CurrentPosition(NULL) 
  { 
  }


  bool XMLReader::Load( std::string fileName, const mitk::DataTreeIteratorBase* it )
  {
   if ( fileName.empty() )
       return false;

   // first test, if file exists, because VTK issues error message if not.
   std::fstream fileExists( fileName.c_str() );
   if (!fileExists) return false;

   XMLReader* xmlReader = XMLReader::New();
   xmlReader->Setm_CurrentPosition( const_cast<mitk::DataTreeIteratorBase*>(it) );
   xmlReader->SetFileName( fileName.c_str() );
   m_FileName = fileName;

   if ( xmlReader->Parse() == 0 )    
   {
     mitk::StatusBar::GetInstance()->DisplayErrorText( "Could not parse xml-Project File!" );
     xmlReader->Delete();  
     return false;
   }

   xmlReader->Build();  
   xmlReader->Delete();  

   DataTreeChangeEvent event;
   it->GetTree()->InvokeEvent( event );

   return true;
  }


  bool XMLReader::GetAttribute( std::string name, std::string& value )
  {
    if ( !m_CurrentNode )
      return false;

    std::string s = m_CurrentNode->GetProperty( name );

    if ( s.empty() )
      return false;
      
    value = s;

    if(name==FILENAME){
      std::string xmlFilePath = itksys::SystemTools::GetProgramPath(m_FileName.c_str());
      value = itksys::SystemTools::CollapseFullPath(s.c_str(), xmlFilePath.c_str());

      // insert a "/" after ":"  necessary for windows when source file and xml file stored on different devices "c:abc"->"c:/abc"
      std::string::size_type e0 = value.find(":");
      if(e0 != std::string::npos){
        std::string::size_type e1 = value.find("/", e0);
        if((e1 != std::string::npos)&&(e1 > e0+1))
          value.insert(e0+1, "/");
      }
      m_SourceFilePath = itksys::SystemTools::GetProgramPath(value.c_str());
      //m_SourceFilePath += "/";
    }
    return true;
  }
  

  bool XMLReader::GetAttribute( std::string name, float& value ) const
  {  
    if ( !m_CurrentNode )
      return false;

    std::string s = m_CurrentNode->GetProperty( name );

    if ( s.empty() )
      return false;

    value = (float) atof( s.c_str() );
    return true;
  }
  

  bool XMLReader::GetAttribute( std::string name, double& value ) const
  {
    if ( !m_CurrentNode )
      return false;

    std::string s = m_CurrentNode->GetProperty( name );

    if ( s.empty() )
      return false;

    value = atof( s.c_str() );
    return true;  
  }
  

  bool XMLReader::GetAttribute( std::string name, int& value ) const
  {
    if ( !m_CurrentNode )
      return false;

    std::string s = m_CurrentNode->GetProperty( name );

    if ( s.empty() )
      return false;

    value = atoi( s.c_str() );

    return true;
  }
  

  bool XMLReader::GetAttribute( std::string name, bool& value ) const
  {
    if ( !m_CurrentNode )
      return false;

    std::string s = m_CurrentNode->GetProperty( name );

    if ( s.empty() )
      return false;

    if ( s == ISTRUE )
      value = true;
    else
      value = false;

    return true;
  }


  bool XMLReader::GetAttribute( std::string name, mitk::Point3D& value ) 
  {
    std::string string;

    if ( !GetAttribute( name, string ) )
      return false;

    int e0 = string.find( "[" );
    int e1 = string.find( ",", e0 + 1 );
    int e2 = string.find( ",", e1 + 1 );
    int e3 = string.find( "]", e2 + 1 );

    value[0] = (mitk::Point3D::ValueType) atof( string.substr( e0 + 1, e1 ).c_str() );
    value[1] = (mitk::Point3D::ValueType) atof( string.substr( e1 + 1, e2 - e1 - 1).c_str() );
    value[2] = (mitk::Point3D::ValueType) atof( string.substr( e2 + 1, e3 - e2 ).c_str() );

    return true;
  }
  bool XMLReader::GetAttribute( std::string name, mitk::Point4D& value ) 
  {
    std::string string;

    if ( !GetAttribute( name, string ) )
      return false;

    int e0 = string.find( "[" );
    int e1 = string.find( ",", e0 + 1 );
    int e2 = string.find( ",", e1 + 1 );
    int e3 = string.find( ",", e2 + 1 );
    int e4 = string.find( "]", e3 + 1 );

    value[0] = (mitk::Point4D::ValueType) atof( string.substr( e0 + 1, e1 ).c_str() );
    value[1] = (mitk::Point4D::ValueType) atof( string.substr( e1 + 1, e2 - e1 - 1).c_str() );
    value[2] = (mitk::Point4D::ValueType) atof( string.substr( e2 + 1, e3 - e2 - 1).c_str() );
    value[3] = (mitk::Point4D::ValueType) atof( string.substr( e3 + 1, e4 - e3 ).c_str() );

    return true;
  }


  bool XMLReader::GetAttribute( std::string name, mitk::Vector3D& value ) 
  {
    mitk::Point3D point;
    GetAttribute( name, point );
    value[0] = point[0];
    value[1] = point[1];
    value[2] = point[2];  
    return true;
  }


  bool XMLReader::GetAttribute( std::string name, itk::Point<int,3>& value ) 
  {
    std::string string;

    if ( !GetAttribute( name, string ) )
      return false;

    int e1 = string.find(",");
    int e2 = string.find(",", e1 + 1);
    int e3 = string.length();

    value[0] = (itk::Point<int,3>::ValueType) atoi( string.substr( 0, e1 ).c_str() );
    value[1] = (itk::Point<int,3>::ValueType) atoi( string.substr( e1 + 1, e2 - e1 - 1).c_str() );
    value[2] = (itk::Point<int,3>::ValueType) atoi( string.substr( e2 + 1, e3 - e2 ).c_str() );

    return true;
  }


  bool XMLReader::GetAttribute( std::string name, mitk::AffineGeometryFrame3D::TransformType& value ) 
  {
    AffineGeometryFrame3D::TransformType::MatrixType matrix;
    AffineGeometryFrame3D::TransformType::OffsetType offset;

    std::string string;

    if ( !GetAttribute( name, string ) )
      return false;

    std::stringstream stream( string );
    char c;

    stream >> c; // [

    for ( int i=0; i<3; i++ )
    {
      stream >> c; // [
      stream >> matrix[i][0];
      stream >> matrix[i][1];
      stream >> matrix[i][2];
      stream >> c; // ]
    }

    stream >> c; // ]

    stream >> c; // [
    stream >> offset[0];
    stream >> offset[1];
    stream >> offset[2];
    stream >> c; // ]

    value.SetMatrix( matrix );
    value.SetOffset( offset );
    return true;
  }


  bool XMLReader::GetAttribute( std::string name, RGBAType& value ) 
  {
    std::string string;

    if ( !GetAttribute( name, string ) )
      return false;

    int e0 = string.find( " " );
    int e1 = string.find( " ", e0 + 2 );
    int e2 = string.find( " ", e1 + 2 );
    int e3 = string.length()-1;

    value[0] = atof( string.substr( 0, e0 ).c_str() );
    value[1] = atof( string.substr( e0 + 2, e1 - e0 - 2).c_str() );
    value[2] = atof( string.substr( e1 + 2, e2 - e1 - 2 ).c_str() );
    value[3] = atof( string.substr( e2 + 2, e3 - e2 - 1 ).c_str() );

    return true;
  }


  bool XMLReader::GetAttribute( std::string name, Color& value ) 
  {
    std::string string;

    if ( !GetAttribute( name, string ) )
      return false;

    int e0 = string.find( " " );
    int e1 = string.find( " ", e0 + 2 );
    int e2 = string.length()-1;

    value[0] = atof( string.substr( 0, e0 ).c_str() );
    value[1] = atof( string.substr( e0 + 2, e1 - e0 - 2).c_str() );
    value[2] = atof( string.substr( e1 + 2, e2 - e1 - 1).c_str() );

    return true;
  }


  void XMLReader::Build()
  {
    if ( !m_Root )
      return;

    ConnectorList::iterator i = m_ConnectorList.begin();
    const ConnectorList::iterator end = m_ConnectorList.end();
    
    while ( i != end )
    {
      m_CurrentNode = (*i).m_XMLNode;
      (*i).m_TreeNode->ReadXMLData( *this );
      i++;
    }

    m_ConnectorList.clear();
  }


  void XMLReader::StartElement( const char *elementName, const char **atts )
  {
    std::string name( elementName );
    XMLNode* node = new XMLNode( elementName, atts );
    
    if ( !m_Root )
    {
      m_Root = node;
      m_FileVersion = atof( m_Root->GetProperty(BaseXMLWriter::FILE_VERSION_KEY).c_str() );
      m_CurrentNode = node;
    }
    else
      m_CurrentNode->Add( node );

    m_CurrentNode = node;

    if ( name == DATA_TREE_NODE ) 
    {
      DataTreeNode::Pointer dataTreeNode = DataTreeNode::New();
      m_CurrentPosition->Add( dataTreeNode );
      int count = m_CurrentPosition->CountChildren();
      m_CurrentPosition->GoToChild( count -1 );
      Connector connector;
      connector.m_TreeNode = dataTreeNode;
      connector.m_XMLNode = node;
      m_ConnectorList.push_back( connector );
    }

  }


  void  XMLReader::EndElement( const char *name )
  {
    m_CurrentNode = m_CurrentNode->GetParent();

    if ( name == TREE_NODE ) 
      m_CurrentPosition->GoToParent();
  }


  std::string mitk::XMLReader::ReadXMLStringAttribut( std::string name, const char** atts ) const
  {
    if(atts)
    {
      const char** attsIter = atts;  

      while(*attsIter) 
      {     
        if ( name == *attsIter )
        {
          attsIter++; 
          return XMLNode::ConvertString( *attsIter );
        }
        attsIter++;
        attsIter++;
      }
    }

      return std::string();
  }


  bool XMLReader::Goto( const std::string& name )
  {
    if ( !m_CurrentNode )
      return false;

    XMLNode::ChildrensType& children = m_CurrentNode->GetChildrens();
    XMLNode::ChildrensType::iterator i = children.begin();
    const XMLNode::ChildrensType::iterator end = children.end();

    while ( i != end )
    {
      if ( (*i)->GetName() == name )
      {
        m_CurrentNode = *i;
        //std::cout << "Position: " << (*i)->GetName() << std::endl;
        return true;
      }
      i++;
    }

    return false;
  }


  bool XMLReader::GotoNext()
  {
    if ( !m_CurrentNode )
      return false;

    std::string name = m_CurrentNode->GetName();
    XMLNode* pos = m_CurrentNode;

    pos = pos->GetParent();

    if ( !pos )
      return false;

    XMLNode::ChildrensType& childrens = pos->GetChildrens();
    XMLNode::ChildrensType::iterator i = childrens.begin();
    const XMLNode::ChildrensType::iterator end = childrens.end();

    while ( i != end && *i != m_CurrentNode ) i++;

    while ( i != end ) {
      if ( (*i)->GetName() == name && *i != m_CurrentNode ) {
        m_CurrentNode = *i;
        //std::cout << "Position: " << (*i)->GetName() << std::endl;
        return true;
      }
      i++;
    }

    return false;
  }


  bool XMLReader::GotoParent()
  {
    if ( !m_CurrentNode )
      return false;

    m_CurrentNode = m_CurrentNode->GetParent();
    //std::cout << "Position: " << m_CurrentNode->GetName() << std::endl;
    return true;
  }


  itk::Object::Pointer XMLReader::CreateObject()
  {
    if ( !m_CurrentNode )
      return NULL;

    return m_CurrentNode->CreateObject();
  }

  //XMLReader
  XMLReader::XMLNode::XMLNode( const char *elementName, const char **atts ) 
    :m_Parent(NULL)
  {
    m_Name = elementName;

    if( atts )
    {
      const char** attsIter = atts;  

      while(*attsIter) 
      {     
        std::string attName = *attsIter;
        attsIter++; 
        std::string attValue = ConvertString( *attsIter );
        attsIter++;
        m_properties[attName] = attValue;
      }
    }
  }


  void XMLReader::XMLNode::Add( XMLNode* node )
  {
    node->m_Parent = this;
    m_Childrens.push_back( node );
  }

  XMLReader::XMLNode* XMLReader::XMLNode::FindNext( const std::string& name, XMLNode* node )
  {
    if ( !node ) 
      return NULL;

    if ( node && node->GetName() == name )
      return node;

    ChildrensType& children = node->GetChildrens();
    ChildrensType::iterator i = children.begin();
    ChildrensType::iterator end = children.end();
    
    while ( i != end )
    {
      XMLNode* result = FindNext( name, *i );

      if ( result )
        return result;

      i++;
    }

    return NULL;
  }

  std::string XMLReader::XMLNode::ConvertString( const char* string )
  {
    std::string buffer( string );

    std::string::size_type pos(0);

    while ( pos != std::string::npos )
    {
      pos = buffer.find( '{', pos );

      if ( pos != std::string::npos )
      {
        buffer[pos] = '<';
      }
    }

    pos = 0;

    while ( pos != std::string::npos )
    {
      pos = buffer.find( '}', pos );

      if ( pos != std::string::npos )
      {
        buffer[pos] = '>';
      }
    }


    return buffer;
  }


  itk::Object::Pointer XMLReader::XMLNode::CreateObject()
  {
    std::string className = GetProperty( XMLIO::CLASS_NAME );

    //std::cout << "create class: " << className << std::endl;

    if ( className.empty() )
      return NULL;

    return mitk::CoreObjectFactory::GetInstance()->CreateCoreObject( className );
  }

  std::string XMLReader::GetSourceFilePath()
  {
    return m_SourceFilePath;
  }

} // namespace mitk

