#ifndef _VTK_POINT_SET_XML_READER__H_
#define _VTK_POINT_SET_XML_READER__H_

#include <mitkPointSet.h>
#include <vtkXMLParser.h>
#include <stack>
#include <list>
#include <string>

namespace mitk
{
class vtkPointSetXMLParser : public vtkXMLParser
{
public:
    typedef std::stack< std::string > ParseStack;
    
    typedef std::list< mitk::PointSet::Pointer > PointSetList;
    
    typedef mitk::PointSet::DataType::PointIdentifier PointIdentifier;
    
    typedef mitk::PointSet::PointType PointType;

    virtual void StartElement (const char *name, const char **atts);
    
    virtual void EndElement (const char *name);
    
    virtual void CharacterDataHandler (const char *inData, int inLength);
    
    virtual mitk::ScalarType ParseScalarType(const char *inData, int inLength);
    
    virtual PointIdentifier ParsePointIdentifier(const char *inData, int inLength);
    
    virtual PointSetList GetParsedPointSets();
    
protected:
    
    ParseStack m_ParseStack;
    
    PointSetList m_PointSetList;    
    
    mitk::PointSet::Pointer m_CurrentPointSet;
    
    PointType m_CurrentPoint;
    
    PointIdentifier m_CurrentPointId;

};    
}
#endif // _VTK_POINT_SET_XML_READER__H_

