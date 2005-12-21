#ifndef MITK_XML_READER
#define MITK_XML_READER

#include <vtkXMLParser.h> 
#include <string>
#include <stack>
#include <mitkDataTree.h>
#include <mitkTransferFunction.h>
#include <mitkGeometry3D.h>
#include <mitkTimeSlicedGeometry.h>
#include <itkRGBAPixel.h>
#include <itkRGBPixel.h>
#include <vtkSystemIncludes.h>

namespace mitk{

  /// Class for parsing a XML file. 
  ///
  /// Class mitkXMLReader is derived from the class vtkXMLParser.
  /// Reads a stream and parses XML element tags and corresponding attributes.
  /// Each element begin tag and its attributes are sent to the StartElement method.
  /// Each element end tag is sent to the EndElement method.
  /// Calls the ObjectFactory (methode CreateObject()) to create objects that are defined in the tags of the XML file.
  /// Don't forget to define new elements in the ObjectFactory!
  class XMLReader : public vtkXMLParser {

	public:
    
    /// Brief Class defines XML nodes
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

      /// replaces characters "{" and "}" with "<" and ">"
      static const char* ConvertString( const char* string );

      static XMLNode* FindNext( const std::string& name, XMLNode* node );
    };

    XMLReader( const mitk::DataTreeIteratorBase* it );

    /// loads the XML file and starts to parse it
    static bool Load( std::string fileName, const mitk::DataTreeIteratorBase* it );

    typedef itk::RGBAPixel< vtkFloatingPointType > 	RGBAType;
    typedef itk::RGBPixel<vtkFloatingPointType> Color;

    /// gets the data as a string of the specified XML attribute
    bool GetAttribute( std::string name, std::string& value ) const;
    
    /// gets the data as a float of the specified XML attribute
    bool GetAttribute( std::string name, float& value ) const;

    /// gets the data as a double of the specified XML attribute
    bool GetAttribute( std::string name, double& value ) const;

    /// gets the data as a integer of the specified XML attribute
    bool GetAttribute( std::string name, int& value ) const;

    /// gets the data as a bool of the specified XML attribute
    bool GetAttribute( std::string name, bool& value ) const;

    /// gets the data as a mitk::Point3D of the specified XML attribute
    bool GetAttribute( std::string name, mitk::Point3D& value ) const;

    /// gets the data as a mitk::Vector3D of the specified XML attribute
    bool GetAttribute( std::string name, mitk::Vector3D& value ) const;

    /// gets the data as a itk::Point of the specified XML attribute
    bool GetAttribute( std::string name, itk::Point<int,3>& value ) const;

    /// todo
    bool GetAttribute( std::string name, mitk::Point4D& value ) const;

    /// gets the data of the specified XML attribute
    bool GetAttribute( std::string name, mitk::AffineGeometryFrame3D::TransformType& value ) const;

    /// gets the data as a RGBAType of the specified XML attribute
    bool GetAttribute( std::string name, RGBAType& value ) const; 

    /// gets the data as a RGBType of the specified XML attribute
    bool GetAttribute( std::string name, Color& value ) const;

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

    /// goes to the specified XML tag
    bool Goto( const std::string& name );
    
    /// goes to the next XML tag
    bool GotoNext();

    /// goes to the parent XML tag
    bool GotoParent();

    /// creates the object that is specified in the XML tag's attribute CLASS_NAME 
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
