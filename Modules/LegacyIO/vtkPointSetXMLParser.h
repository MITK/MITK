/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _VTK_POINT_SET_XML_READER__H_
#define _VTK_POINT_SET_XML_READER__H_

#include <MitkLegacyIOExports.h>
#include <mitkPointSet.h>
#include <vtkXMLParser.h>
#include <stack>
#include <list>
#include <string>


namespace mitk
{

/**
 * @brief Implementation of the vtkXMLParser interface for reading mitk::PointSets.
 *
 * This class implements the XMLParser interface of the vtkXMLParser which is based
 * on expat. It is used by the mitk::PointSetReader and is NOT INTENDED TO BE USED
 * FROM THE END-USER. If you want to read point sets, use the mitk::PointSetReader.
 * @ingroup Process
 *
 * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
 */
class DEPRECATED() MitkLegacyIO_EXPORT vtkPointSetXMLParser : public vtkXMLParser
{
public:
    vtkTypeMacro(vtkPointSetXMLParser,vtkXMLParser);

    static vtkPointSetXMLParser* New();

    typedef mitk::PointSet PointSetType;

    typedef std::stack< std::string > ParseStack;

    typedef std::list< PointSetType::Pointer > PointSetList;

    typedef PointSetType::DataType::PointIdentifier PointIdentifier;

    typedef PointSetType::PointType PointType;

    virtual int InitializeParser();
    virtual int CleanupParser();
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
     * @param inData a char array containing the parsed string data
     * @param inLength the length of the parsed data string.
     */
    virtual void CharacterDataHandler (const char *inData, int inLength);

    /**
     * Converts the given data to mitk::ScalarType.
     */
    virtual mitk::ScalarType ParseScalarType(const std::string &data);

    /**
     * Converts the given data to an PointIdentifier
     */
    virtual PointIdentifier ParsePointIdentifier(const std::string &data);

    /**
     * @returns the list of point sets which have been read from file.
     * NOTE: your have to call the Parse() function, before this function.
     */
    virtual PointSetList GetParsedPointSets();

protected:
    vtkPointSetXMLParser();
    virtual ~vtkPointSetXMLParser();

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
    PointSetType::Pointer m_CurrentPointSet;

    /**
     * The current point which is processed
     * by the parser.
     */
    PointType m_CurrentPoint;

    std::string m_CurId;
    std::string m_CurXString;
    std::string m_CurYString;
    std::string m_CurZString;

    /**
     * The current point id which is processed
     * by the parser.
     */
    PointIdentifier m_CurrentPointId;

    std::locale m_PreviousLocale;

};
}
#endif // _VTK_POINT_SET_XML_READER__H_
