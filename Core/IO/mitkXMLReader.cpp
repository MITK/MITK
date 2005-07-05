#include "mitkXMLReader.h"

#include <vtkXMLDataElement.h>
#include <mitkStatusBar.h>

#include <fstream>
#include <string>
#include <stack>
#include <stdio.h>

namespace mitk {

  static const std::string XMLReader::DATA;
  static const std::string XMLReader::TREE_NODE = "treeNode";
  static const std::string XMLReader::CLASS_NAME = "CLASS_NAME";
  const std::string mitk::StateMachineFactory::ISTRUE = "TRUE";
  const std::string mitk::StateMachineFactory::ISFALSE = "FALSE";

  /**
   *
   */
  XMLReader::XMLReader()
  {
  }

  /**
   *
   */
  bool XMLReader::Load( std::string fileName, mitk::DataTreeIteratorBase* it )
  {
   if ( fileName.empty() )
       return false;

   XMLReader* xmlReader = new XMLReader();
   bool result = xmlReader->SetFileName( fileName.c_str() );

   if ( xmlReader->Parse() == 0 )    
   {
     mitk::StatusBar::DisplayErrorText( "Could not parse xml-Project File!" );
     result = false;
   }
  
   xmlReader->Delete();
   return result;
  }

  /**
   *
   */
  void  XMLReader::StartElement( const char *elementName, const char **atts )
  {

    std::string name(elementName);

	  if ( name == DATA ) 					
    { 
      std::string className = ReadXMLStringAttribut( CLASS_NAME, atts ); 
      mitk::BaseData::Pointer data = ObjectFactory::CreatObject( className );
      data->ReadXML( this );
    }     
    else if ( name == TREE_NODE )   
    {

    }
  }

  /**
   *
   */
  void  XMLReader::EndElement( const char *elementName )
  {
  }

  // help

  /**
   *
   */
std::string mitk::XMLReader::ReadXMLStringAttribut( std::string name, const char** atts )
{
  if(atts)
    {
     const char** attsIter = atts;  

     while(*attsIter) 
     {     
       if ( name == *attsIter )
       {
        attsIter++;      
        return *attsIter;
       }
      attsIter++;
      attsIter++;
     }
    }

    return std::string();
}

  /**
   *
   */
  int mitk::XMLReader::ReadXMLIntegerAttribut( std::string name, const char** atts )
  {
    std::string s = ReadXMLStringAttribut( name, atts );
    return atoi( s.c_str() );
  }

  /**
   *
   */
  float mitk::XMLReader::ReadXMLFloatAttribut( std::string name, const char** atts )
  {
    std::string s = ReadXMLStringAttribut( name, atts );
    return (float) atof( s.c_str() );
  }

  /**
   *
   */
  double mitk::XMLReader::ReadXMLDoubleAttribut( std::string name, const char** atts )
  {
    std::string s = ReadXMLStringAttribut( name, atts );
    return atof( s.c_str() );
  }

  /**
   *
   */
  bool mitk::XMLReader::ReadXMLBooleanAttribut( std::string name, const char** atts )
  {
    std::string s = ReadXMLStringAttribut( name, atts );

    if ( s == ISTRUE )
      return true;
    else
      return false;
  }

} // namespace mitk

