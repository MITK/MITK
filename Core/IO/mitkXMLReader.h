#ifndef MITK_XML_READER
#define MITK_XML_READER

#include <vtkXMLParser.h> 
#include <string>
#include <stack>
#include <mitkDataTree.h>
#include <mitkTransferFunction.h>
#include <mitkGeometry3D.h>
#include <mitkTimeSlicedGeometry.h>

namespace mitk{

//##Documentation
//## @brief Parses the XML file. Class is derived from the class vtkXMLParser.
//## Reads a stream and parses XML element tags and corresponding attributes.
//## Each element begin tag and its attributes are sent to the StartElement method.
//## Each element end tag is sent to the EndElement method.
//##
//## Calls the ObjectFactory (methode CreateObject()) to create objects that are defined in the tags of the XML file.
//## Don't forget to define the new elements in the ObjectFactory!

  class XMLReader : public vtkXMLParser {

	public:

    class XMLNode
    {
    public:
      typedef std::vector<XMLNode*> ChildrensType;

    private:
      typedef std::map<std::string,std::string> PropertyType;
      std::string m_Name;
      PropertyType m_properties;
      ChildrensType m_Childrens;
      XMLNode* m_Parent;

    public: 
      XMLNode( const char *elementName, const char **atts );
      const std::string& GetName(){ return m_Name; };
      const std::string& GetProperty( const std::string& key ) { return m_properties[key]; };
      ChildrensType& GetChildrens() { return m_Childrens; };
      itk::Object::Pointer CreateObject();


      void Add( XMLNode* node );
      XMLNode* GetParent() { return m_Parent; };
      static const char* ConvertString( const char* string );
      static XMLNode* FindNext( const std::string& name, XMLNode* node );
    };

    XMLReader( const mitk::DataTreeIteratorBase* it );

    //##Documentation
    //## @brief loads the XML file and starts to parse it
    static bool Load( std::string fileName, const mitk::DataTreeIteratorBase* it );

    bool GetAttribute( std::string name, std::string& value ) const;
    bool GetAttribute( std::string name, float& value ) const;
    bool GetAttribute( std::string name, double& value ) const;
    bool GetAttribute( std::string name, int& value ) const;
    bool GetAttribute( std::string name, bool& value ) const;
    bool GetAttribute( std::string name, mitk::Point3D& value ) const;
    bool GetAttribute( std::string name, mitk::Vector3D& value ) const;
    bool GetAttribute( std::string name, itk::Point<int,3>& value ) const;
    bool GetAttribute( std::string name, mitk::Point4D& value ) const;
    bool GetAttribute( std::string name, mitk::AffineGeometryFrame3D::TransformType& value ) const;

    static const std::string DATA;
    static const std::string TREE_NODE;
    static const std::string DATA_TREE_NODE;
    static const std::string ISTRUE;
    static const std::string ISFALSE;
    static const std::string PROPERTY;
    static const std::string PROPERTY_LIST;
    static const std::string DATA_TREE; 
    static const std::string RENDERER_NAME;
    static const std::string PROPERTY_KEY;
    static const std::string RENDERER;
    static const std::string FILENAME;
    static const std::string MIN;
    static const std::string MAX;
    static const std::string VALID;

    //##Documentation
    //## @brief goes to the specified XML tag
    bool Goto( const std::string& name );
    
    //##Documentation
    //## @brief goes to the next XML tag
    bool GotoNext();

    //##Documentation
    //## @brief goes to the parent XML tag
    bool GotoParent();

    //##Documentation
    //## @brief creates the object that is specified in the XML tag's attribute CLASS_NAME 
    itk::Object::Pointer CreateObject();

  protected:
    void  StartElement (const char *elementName, const char **atts);
    void  EndElement (const char *elementName);
    void Build();

    XMLNode* m_Root;
    XMLNode* m_CurrentNode;

    struct Connector{ 
      DataTreeNode* m_TreeNode;
      XMLNode* m_XMLNode;
    };

    typedef std::vector<Connector> ConnectorList;
    ConnectorList m_ConnectorList;
    mitk::DataTreeIteratorBase* m_CurrentPosition;


    std::string ReadXMLStringAttribut( std::string name, const char** atts) const;
	};
}
#endif
