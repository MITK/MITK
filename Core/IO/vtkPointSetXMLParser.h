#ifndef _VTK_POINT_SET_XML_READER__H_
#define _VTK_POINT_SET_XML_READER__H_

#include <mitkPointSet.h>
#include <vtkXMLParser.h>
#include <stack>
#include <list>
#include <string>


namespace mitk
{
    
/**
 * @brief Implementation of the vtkXMLParser interface for reading mitk::PointSets.
 * @ingroup process
 * This class implements the XMLParser interface of the vtkXMLParser which is based
 * on expat. It is used by the mitk::PointSetReader and is NOT INTENDED TO BE USED 
 * FROM THE END-USER. If you want to read point sets, use the mitk::PointSetReader.
 */
class vtkPointSetXMLParser : public vtkXMLParser
{
public:
    typedef std::stack< std::string > ParseStack;
    
    typedef std::list< mitk::PointSet::Pointer > PointSetList;
    
    typedef mitk::PointSet::DataType::PointIdentifier PointIdentifier;
    
    typedef mitk::PointSet::PointType PointType;

    /**
     * Handler function which is called, when a new xml start-tag 
     * has been parsed.
     */
    virtual void StartElement (const char *name, const char **atts);
    
    /**
     * Handler function which is called, when a xml end-tag 
     * has been parsed.
     */
    virtual void EndElement (const char *name);
    
    /**
     * Handler function which is called, if characted data has been 
     * parsed by expat.
     */
    virtual void CharacterDataHandler (const char *inData, int inLength);
    
    /**
     * Converts the given data to mitk::ScalarType.
     * @param inData a char array containing the parsed string data
     * @param inLength the length of the parsed data string.
     */
    virtual mitk::ScalarType ParseScalarType(const char *inData, int inLength);
    
    /**
     * Converts the given data to an PointIdentifier
     * @param inData a char array containing the parsed string data
     * @param inLength the length of the parsed data string.
     */
    virtual PointIdentifier ParsePointIdentifier(const char *inData, int inLength);
    
    /**
     * @returns the list of point sets which have been read from file.
     * NOTE: your have to call the Parse() function, before this function.
     */
    virtual PointSetList GetParsedPointSets();
    
protected:
    
    /**
     * A stack containing the parsed start-tags. 
     * If an end tag is encountered, it is matched with the
     * top element of the stack. 
     */
    ParseStack m_ParseStack;
    
    /**
     * Contains the parsed point sets.
     */
    PointSetList m_PointSetList;    
    
    /** 
     * The current point set which is processed
     * by the parser.
     */
    mitk::PointSet::Pointer m_CurrentPointSet;
    
    /** 
     * The current point which is processed
     * by the parser.
     */
    PointType m_CurrentPoint;
    
    /** 
     * The current point id which is processed
     * by the parser.
     */
    PointIdentifier m_CurrentPointId;

};    
}
#endif // _VTK_POINT_SET_XML_READER__H_

