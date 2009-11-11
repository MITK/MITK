/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12. Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PlanarFigureReader__H_
#define _MITK_PlanarFigureReader__H_

#include <mitkPlanarFigureSource.h>
#include <mitkFileReader.h>

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
class MITKEXT_CORE_EXPORT PlanarFigureReader: public PlanarFigureSource, public FileReader
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

    /**
    * Allocates a new dummy output object and returns it. 
    * GenerateData() will overwrite the outputs with specific types, 
    * @param idx the index of the output for which an object should be created
    * @returns the dummy object
    */
    virtual DataObjectPointer MakeOutput ( unsigned int idx );

protected:

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

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;  
    bool m_Success;  
};   
}
#endif
