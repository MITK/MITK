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

#ifndef MITK_BASE_XML_WRITER
#define MITK_BASE_XML_WRITER

#include <fstream>
#include <string>
#include <stack>
#include <sstream>

#include "mitkCommon.h"

namespace mitk{

  //##Documentation
  //## @brief Base class for writing XML nodes and XML attributes.
  //##
  //## Implements methods for writing a XML file (XML nodes and XML attributes).
  //## The data is stored with XML attributes.
  //## Function BeginNode() writes a XML node.
  //## The functions WriteProperty() are used to write XML attributes.
  //## Function EndNode() closes an open XML node tag.
  //##
  //## The XMLIO class manages the writing and reading of the data.
  //## An example of how to start the XMLWriter can be found in the function Save() of the DataTree class.
  //## @ingroup IO
  class MITK_CORE_EXPORT BaseXMLWriter {
    
    /// encapsulates all information of a node in a stream
    class MITK_CORE_EXPORT StreamNode
    {
    public:
      StreamNode( std::string name ): m_Name(name){};
      std::string GetName() { return m_Name; };
      std::ostream& GetPropertyStream() { return m_PropertyStream; };
      std::ostream& GetChildrenStream() { return m_ChildrenStream; };
      std::ostream& GetComment() { return m_Comment; };      

      void Write( std::ostream& out, int steps );
    private:
      std::string m_Name;
      std::stringstream m_PropertyStream;
      std::stringstream m_ChildrenStream;
      std::stringstream m_Comment;
    };

    std::stack<StreamNode*> m_Stack;
    std::ostream* m_Out;
    int m_Increase;
    int m_Space;
    int m_NodeCount;
    bool m_File;
    bool m_FirstNode;

  public:

    /// constructor (output to file)
    BaseXMLWriter( const char* filename, int space = 3);

    /// constructor (output to stringstream)
    //BaseXMLWriter( std::ostringstream& out, int space = 3 );

    /// destructor
    virtual ~BaseXMLWriter();

    /// begins a new XML node
    // parameter name specifies the name of the XML node
    void BeginNode( const std::string& name );

    /// closes an open XML node
    void EndNode( );

    /// writes a XML attribute that datatype is a string
    void WriteProperty( const std::string& key, const std::string& value ) const;

    /// writes a comment in the XML file
    void WriteComment( const std::string& key );
  
    /// returns the current node depth
    int GetCurrentDeph() const;

    /// returns the current count of nodes
    int GetNodeCount() const;

    /// returns the indention space of the XML nodes
    int GetSpace() const;

    /// sets the indention space of the XML nodes
    void SetSpace( int space );

    /// returns the path and the filename of the XML file
    const std::string GetXMLPathAndFileName();

    static const std::string FILE_VERSION_KEY;
    static const std::string VERSION;

protected:

    /// replaces characters "<" and ">" with "{" and "}"
    const char* ConvertString( const char* string ) const;
    const char* m_XMLPathAndFilename;
  };
}
#endif // MITK_BASE_XML_WRITER
