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


#ifndef _MITK_PlanarFigureReader__H_
#define _MITK_PlanarFigureReader__H_

#include "mitkPlanarFigureSource.h"
#include "PlanarFigureExports.h"
#include "mitkFileReader.h"

#include <list>


class TiXmlElement;
namespace mitk
{
/**
 * @brief reads xml representations of mitk::PlanarFigure from a file
 *
 * Reader for xml files containing one or multiple xml represenations of
 * mitk::PlanarFigure. If multiple mitk::PlanarFigure are stored in one file,
 * these are assigned to multiple outputs of the filter.
 * @ingroup IO
*/
class PlanarFigure_EXPORT PlanarFigureReader: public PlanarFigureSource, public FileReader
{
public:

    mitkClassMacro( PlanarFigureReader, FileReader );

    itkNewMacro( Self );

    /**
     * @brief Sets the filename of the file to be read
     * @param _arg the filename of the point set xml-file
     */
    itkSetStringMacro( FileName );

    /**
     * @brief Returns the filename of the point set xml-file.
     * @returns the filename of the point set xml-file.
     */
    itkGetStringMacro( FileName );

    /**
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro( FilePrefix );

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro( FilePrefix );

    /**
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro( FilePattern );

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro( FilePattern );

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

    /**
     * @returns whether the last read attempt was successful or not.
     */
    itkGetConstMacro(Success, bool);

protected:

    typedef std::list< double > DoubleList;

    /**
     * Constructor
     */
    PlanarFigureReader();

    /**
     * Virtual destructor
     */
    virtual ~PlanarFigureReader();

    /**
     * Actually reads the point sets from the given file
     */
    virtual void GenerateData();

    /**
     * Does nothing in the current implementation
     */
    virtual void GenerateOutputInformation();

    /**
     * Resizes the output-objects according to the given number.
     * @param num the new number of output objects.
     */
    virtual void ResizeOutputs( const unsigned int& num );

    /**
     * Checks if the given file has appropriate
     * read access.
     * @returns true if the file exists and may be read
     *          or false otherwise.
     */
    virtual int CanReadFile (const char *name);

    /**
    * \brief parses the element for the attributes x,y,z and returns a mitk::Vector3D filled with these values
    * \param[in] e the TiXmlElement that will be parsed
    * \return returns a mitk::Vector3D with the values x,y,z
    */
    mitk::Vector3D GetVectorFromXMLNode(TiXmlElement* e);

    /**
    * \brief parses the element for the attributes x,y,z and returns a mitk::Point3D filled with these values
    * \param[in] e the TiXmlElement that will be parsed
    * \return returns a mitk::Point3D with the values x,y,z
    */
    mitk::Point3D GetPointFromXMLNode(TiXmlElement* e);

    /**
    * \brief parses the element for the attributes name0 to nameN, where "name" and the number of attributes
    * to read are passed as argument. Returns a list of double vales.
    * \param[in] e the TiXmlElement that will be parsed
    * \param[in] attributeNameBase the basic name of the parameters
    * \param[in] count the number of parameters
    * \return returns a mitk::Point3D with the values x,y,z
    */
    DoubleList GetDoubleAttributeListFromXMLNode(TiXmlElement* e, const char *attributeNameBase, unsigned int count);


    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;
    bool m_Success;
};
}
#endif
