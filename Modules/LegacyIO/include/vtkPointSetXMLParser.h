/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkPointSetXMLParser_h
#define vtkPointSetXMLParser_h

#include <MitkLegacyIOExports.h>
#include <list>
#include <mitkPointSet.h>
#include <stack>
#include <string>
#include <vtkXMLParser.h>

namespace mitk
{
  /**
   * \brief Implementation of the vtkXMLParser interface for reading mitk::PointSets.
   *
   * This class implements the XMLParser interface of the vtkXMLParser which is based
   * on expat. It is used by the mitk::PointSetReader and is NOT INTENDED TO BE USED
   * FROM THE END-USER. If you want to read point sets, use the mitk::PointSetReader.
   *
   * \ingroup MitkLegacyIOModule
   * \deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
   */
  class DEPRECATED() MITKLEGACYIO_EXPORT vtkPointSetXMLParser : public vtkXMLParser
  {
  public:
    vtkTypeMacro(vtkPointSetXMLParser, vtkXMLParser);

    /** \brief Create a new vtkPointSetXMLParser instance. */
    static vtkPointSetXMLParser *New();

    typedef mitk::PointSet PointSetType;       ///< \brief The point set type.

    typedef std::stack<std::string> ParseStack; ///< \brief Stack type for tracking parsed XML tags.

    typedef std::list<PointSetType::Pointer> PointSetList; ///< \brief List type for storing parsed point sets.

    typedef PointSetType::DataType::PointIdentifier PointIdentifier; ///< \brief The point identifier type.

    typedef PointSetType::PointType PointType; ///< \brief The point type.

    /** \brief Initialize the XML parser.
     * \return 1 on success, 0 on failure.
     */
    int InitializeParser() override;
    /** \brief Clean up the XML parser.
     * \return 1 on success, 0 on failure.
     */
    int CleanupParser() override;
    /**
     * \brief Handler function which is called when a new XML start-tag
     * has been parsed.
     * \param name The name of the start-tag.
     * \param atts The attributes of the start-tag as name/value pairs.
     */
    void StartElement(const char *name, const char **atts) override;

    /**
     * \brief Handler function which is called when an XML end-tag
     * has been parsed.
     * \param name The name of the end-tag.
     */
    void EndElement(const char *name) override;

    /**
     * \brief Handler function called when character data has been parsed by expat.
     * \param[in] inData A char array containing the parsed string data.
     * \param[in] inLength The length of the parsed data string.
     */
    void CharacterDataHandler(const char *inData, int inLength) override;

    /**
     * \brief Convert the given string data to mitk::ScalarType.
     * \param data The string to convert.
     * \return The parsed scalar value.
     */
    virtual mitk::ScalarType ParseScalarType(const std::string &data);

    /**
     * \brief Convert the given string data to a PointIdentifier.
     * \param data The string to convert.
     * \return The parsed point identifier.
     */
    virtual PointIdentifier ParsePointIdentifier(const std::string &data);

    /**
     * \brief Get the list of point sets which have been read from file.
     *
     * \note You must call Parse() before calling this function.
     * \return The list of parsed point sets.
     */
    virtual PointSetList GetParsedPointSets();

  protected:
    /** \brief Default constructor. */
    vtkPointSetXMLParser();
    /** \brief Destructor. */
    ~vtkPointSetXMLParser() override;

    /**
     * \brief A stack containing the parsed start-tags.
     *
     * If an end tag is encountered, it is matched with the
     * top element of the stack.
     */
    ParseStack m_ParseStack;

    /**
     * \brief Contains the parsed point sets.
     */
    PointSetList m_PointSetList;

    /**
     * \brief The current point set which is processed
     * by the parser.
     */
    PointSetType::Pointer m_CurrentPointSet;

    /**
     * \brief The current point which is processed
     * by the parser.
     */
    PointType m_CurrentPoint;

    std::string m_CurId;       ///< \brief The current point ID string.
    std::string m_CurXString;  ///< \brief The current X coordinate string.
    std::string m_CurYString;  ///< \brief The current Y coordinate string.
    std::string m_CurZString;  ///< \brief The current Z coordinate string.

    /**
     * \brief The current point id which is processed
     * by the parser.
     */
    PointIdentifier m_CurrentPointId;

    std::locale m_PreviousLocale; ///< \brief The locale saved before parsing.
  };
}
#endif // vtkPointSetXMLParser_h
