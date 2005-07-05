#ifndef MITK_XML_WRITER
#define MITK_XML_WRITER

#include <vtkXMLParser.h> 
#include <string>


namespace mitk{

	class XMLReader : public vtkXMLParser {

	public:
    XMLReader();
    static bool Load( std::string fileName, mitk::DataTreeIteratorBase* it );
    std::string GetStringAttribut();
    float GetFloatAttribut( std::string name );
    double GetDoubleAttribut( std::string name );
    int GetIntegerAttribut( std::string name );
    bool GetBooleanAttribut( std::string name );

  protected:
    void  StartElement (const char *elementName, const char **atts);
    void  EndElement (const char *elementName);

    std::string ReadXMLStringAttribut( std::string name, const char** atts);
    float ReadXMLFloatAttribut( std::string name, const char** atts );
    double ReadXMLDoubleAttribut( std::string name, const char** atts );
    int ReadXMLIntegerAttribut( std::string name, const char** atts );
    bool ReadXMLBooleanAttribut( std::string name, const char** atts );

    static const std::string DATA;
    static const std::string TREE_NODE;
    static const std::string CLASS_NAME;
    static const std::string ISTRUE;
    static const std::string ISFALSE;

	};
}
#endif
