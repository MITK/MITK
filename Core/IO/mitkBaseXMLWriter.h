#ifndef MITK_BASE_XML_WRITER
#define MITK_BASE_XML_WRITER

#include <fstream>
#include <string>
#include <stack>
#include <sstream>

namespace mitk{

//##Documentation
//## @brief Base class for the XML writer.
//## Implements methods for writing a XML file.
//##
//## Note - the functions WriteProperty() are used to write XML attributes.
//##
//## This XML writer concept supports to store the data with XML attributes.

  class BaseXMLWriter {

    class StreamNode
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

    /**
     * Construktor
     */
    BaseXMLWriter( const char* filename, int space = 3);

    /**
     * Construktor
     */
    BaseXMLWriter( std::ostream& out, int space = 3 );

    /**
     * Destruktor
     */
    virtual ~BaseXMLWriter();

    /*
     * begins a new XML node
     */
    void BeginNode( const std::string& name );

    /**
     * closes an open XML node
     */
    void EndNode( );

    /**
     * Writes a string XML attribute
     */
    void WriteProperty( const std::string& key, const char* value ) const;

    /**
     * Writes a string XML attribute
     */
    void WriteProperty( const std::string& key, const std::string& value ) const;

    /**
     * Writes a int XML attribute
     */
    void WriteProperty( const std::string& key, int value ) const;

    /**
     * Writes a float XML attribute
     */
    void WriteProperty( const std::string& key, float value ) const;

    /**
     * Writes a double XML attribute
     */
    void WriteProperty( const std::string& key, double value ) const;

    /**
     * Writes a bool XML attribute
     */
    void WriteProperty( const std::string& key, bool value ) const;

    /**
     * Writes a comment
     */
    void WriteComment( const std::string& key );
  
    /**
     * retun the current deph
     */
    int GetCurrentDeph() const;

    /**
     * Get the current count of nodes
     */
    int GetNodeCount() const;

    /**
     * Get the space
     */
    int GetSpace() const;

    /**
     *
     */
    void SetSpace( int space );

protected:
    /**
     * replace char < and > through { and }
     */
    const char* ConvertString( const char* string ) const;
  };
}
#endif // MITK_BASE_XML_WRITER
